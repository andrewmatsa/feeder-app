-- AquaFeed: Expo push token storage for mobile push notifications
-- Run in: Supabase Dashboard -> SQL Editor

CREATE TABLE IF NOT EXISTS public.push_tokens (
    id         UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id    UUID NOT NULL REFERENCES auth.users(id) ON DELETE CASCADE,
    device_id  UUID NOT NULL REFERENCES public.devices(id) ON DELETE CASCADE,
    token      TEXT NOT NULL,
    platform   TEXT NOT NULL DEFAULT 'expo',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE (device_id, token)
);

ALTER TABLE public.push_tokens ENABLE ROW LEVEL SECURITY;

CREATE POLICY "Users manage own push tokens" ON public.push_tokens
    FOR ALL USING (auth.uid() = user_id) WITH CHECK (auth.uid() = user_id);

-- Per-device alert dedupe state (avoid re-notifying every poll while a
-- condition stays bad); reuses the existing `settings` JSONB column added
-- in 004_device_settings.sql rather than adding more narrow columns.
-- Shape written by backend/push_notifications.py: {"alerts": {"lowBattery": "<iso ts>", "offline": "<iso ts>"}}
