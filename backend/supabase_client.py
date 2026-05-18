"""Supabase client singleton."""

from supabase import Client, create_client

try:
    from .config import SUPABASE_ANON_KEY, SUPABASE_URL
except ImportError:
    from config import SUPABASE_ANON_KEY, SUPABASE_URL

_client: Client | None = None


def get_supabase() -> Client:
    global _client
    if _client is None:
        if not SUPABASE_URL or not SUPABASE_ANON_KEY:
            raise RuntimeError("SUPABASE_URL and SUPABASE_ANON_KEY must be set in environment")
        _client = create_client(SUPABASE_URL, SUPABASE_ANON_KEY)
    return _client
