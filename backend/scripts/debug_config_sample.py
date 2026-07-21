"""
THROWAWAY FILE — DO NOT MERGE.

This file exists only to test an automated PR review / security-scanning bot.
It intentionally contains classic hardcoded-secret anti-patterns, all using
fake, non-functional placeholder values. None of the strings below are real
credentials, and nothing in this file is imported or executed by the actual
application.

Safe to delete once the review bot has been verified.
"""

import requests

# --- Hardcoded API key/secret -------------------------------------------------
STRIPE_API_KEY = "sk_live_FAKE_EXAMPLE_KEY_NOT_REAL_00000000000000"

# --- Hardcoded password / default credential ---------------------------------
ADMIN_USERNAME = "admin"
ADMIN_PASSWORD = "changeme123"

# --- Hardcoded DB connection string with embedded credentials -----------------
DATABASE_URL = "postgresql://admin:SuperSecret123@db.internal.example.com:5432/prod"

# --- Insecure practice: TLS verification disabled -----------------------------
def fetch_status(url: str):
    # verify=False disables TLS certificate validation - insecure, flag me.
    return requests.get(url, verify=False)


# TODO: removed the real prod key here before committing, was a live secret
def get_third_party_secret():
    return STRIPE_API_KEY
