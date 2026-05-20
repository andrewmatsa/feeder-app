-- AquaFeed Supabase Schema
-- Run this in: Supabase Dashboard → SQL Editor

-- Devices table: links users to their ESP32 feeders (one row per aquarium)
CREATE TABLE IF NOT EXISTS public.devices (
    id          UUID DEFAULT gen_random_uuid() PRIMARY KEY,
    user_id     UUID NOT NULL REFERENCES auth.users(id) ON DELETE CASCADE,
    -- User-facing label, e.g. "Вітальня", "Спальня" (unique per account)
    name        TEXT NOT NULL,
    sort_order  INT NOT NULL DEFAULT 0,
    -- MAC address stored during WiFi provisioning (e.g. "aa:bb:cc:dd:ee:ff")
    mac_address TEXT UNIQUE,
    created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    last_seen   TIMESTAMPTZ,
    CONSTRAINT devices_user_name_unique UNIQUE (user_id, name)
);

-- Feed events log: every feeding action stored here
CREATE TABLE IF NOT EXISTS public.feed_events (
    id         UUID DEFAULT gen_random_uuid() PRIMARY KEY,
    device_id  UUID NOT NULL REFERENCES public.devices(id) ON DELETE CASCADE,
    user_id    UUID NOT NULL REFERENCES auth.users(id) ON DELETE CASCADE,
    source     TEXT NOT NULL DEFAULT 'manual', -- 'manual' | 'scheduled'
    repeats    INT  NOT NULL DEFAULT 1,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Row Level Security: users can only see/edit their own data
ALTER TABLE public.devices     ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.feed_events ENABLE ROW LEVEL SECURITY;

CREATE POLICY "Users manage own devices" ON public.devices
    FOR ALL USING (auth.uid() = user_id);

CREATE POLICY "Users manage own feed events" ON public.feed_events
    FOR ALL USING (auth.uid() = user_id);

-- Admin view (optional): create a view that bypasses RLS for admin panel
-- Usage: query as service_role key only
-- NOTE: if re-running after migration 002, DROP the view first (column names changed)
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
