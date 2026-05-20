-- AquaFeed: Per-device settings storage and real-device endpoint routing
-- Run in: Supabase Dashboard → SQL Editor

ALTER TABLE public.devices
    ADD COLUMN IF NOT EXISTS endpoint_url TEXT,
    ADD COLUMN IF NOT EXISTS settings     JSONB NOT NULL DEFAULT '{}';
