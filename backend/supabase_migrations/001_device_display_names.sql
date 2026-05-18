-- Migration for existing Supabase projects (run after initial schema)
-- Adds per-account unique aquarium names and sort order.

ALTER TABLE public.devices
    ADD COLUMN IF NOT EXISTS sort_order INT NOT NULL DEFAULT 0;

-- Drop generic default so new rows get names from the API
ALTER TABLE public.devices
    ALTER COLUMN name DROP DEFAULT;

-- Resolve duplicate names before adding unique constraint (manual fix may be needed)
-- UPDATE public.devices SET name = name || ' (' || left(id::text, 8) || ')' WHERE ...

DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM pg_constraint WHERE conname = 'devices_user_name_unique'
    ) THEN
        ALTER TABLE public.devices
            ADD CONSTRAINT devices_user_name_unique UNIQUE (user_id, name);
    END IF;
END $$;
