-- AquaFeed: Profiles table, role trigger, and updated admin_users view
-- Run in: Supabase Dashboard → SQL Editor

-- 1. Profiles table (one row per auth.users row)
CREATE TABLE IF NOT EXISTS public.profiles (
    id         UUID PRIMARY KEY REFERENCES auth.users(id) ON DELETE CASCADE,
    role       TEXT NOT NULL DEFAULT 'user' CHECK (role IN ('admin', 'user')),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

ALTER TABLE public.profiles ENABLE ROW LEVEL SECURITY;

-- Users can read their own profile (e.g. /auth/me)
CREATE POLICY "Users read own profile" ON public.profiles
    FOR SELECT USING (auth.uid() = id);

-- 2. Auto-create profile row on new signup
CREATE OR REPLACE FUNCTION public.handle_new_user()
RETURNS TRIGGER
LANGUAGE plpgsql
SECURITY DEFINER
SET search_path = public
AS $$
BEGIN
    INSERT INTO public.profiles (id, role)
    VALUES (NEW.id, 'user')
    ON CONFLICT (id) DO NOTHING;
    RETURN NEW;
END;
$$;

DROP TRIGGER IF EXISTS on_auth_user_created ON auth.users;
CREATE TRIGGER on_auth_user_created
    AFTER INSERT ON auth.users
    FOR EACH ROW EXECUTE FUNCTION public.handle_new_user();

-- 3. Back-fill profiles for existing users (safe to run multiple times)
INSERT INTO public.profiles (id, role)
SELECT id, 'user' FROM auth.users
ON CONFLICT (id) DO NOTHING;

-- 4. Rebuild admin_users view: adds role + last_activity from feed_events
DROP VIEW IF EXISTS public.admin_users;
CREATE VIEW public.admin_users AS
    SELECT
        u.id,
        u.email,
        u.created_at,
        COALESCE(p.role, 'user')                             AS role,
        COUNT(DISTINCT d.id)                                 AS device_count,
        GREATEST(MAX(d.last_seen), MAX(fe.created_at))       AS last_activity
    FROM auth.users u
    LEFT JOIN public.profiles    p  ON p.id  = u.id
    LEFT JOIN public.devices     d  ON d.user_id  = u.id
    LEFT JOIN public.feed_events fe ON fe.user_id = u.id
    GROUP BY u.id, u.email, u.created_at, p.role;

-- After running, promote your account to admin:
-- UPDATE public.profiles SET role = 'admin' WHERE id = '<your-user-uuid>';
