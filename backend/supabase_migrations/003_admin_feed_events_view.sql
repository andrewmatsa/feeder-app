-- AquaFeed: Admin feed events view for the audit journal
-- Run in: Supabase Dashboard → SQL Editor

DROP VIEW IF EXISTS public.admin_feed_events;

CREATE VIEW public.admin_feed_events AS
    SELECT
        fe.id,
        fe.created_at,
        fe.source,
        fe.repeats,
        u.email   AS user_email,
        d.name    AS device_name
    FROM public.feed_events fe
    JOIN public.devices     d ON d.id = fe.device_id
    JOIN auth.users         u ON u.id = fe.user_id
    ORDER BY fe.created_at DESC;
