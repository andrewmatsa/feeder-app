"""Authentication router using Supabase Auth."""

from fastapi import APIRouter, Depends, HTTPException, status
from gotrue.types import User
from pydantic import BaseModel, EmailStr

try:
    from .dependencies import get_current_user
    from .supabase_client import get_supabase
except ImportError:
    from dependencies import get_current_user
    from supabase_client import get_supabase

router = APIRouter(prefix="/auth", tags=["auth"])


class RegisterRequest(BaseModel):
    email: EmailStr
    password: str


class LoginRequest(BaseModel):
    email: EmailStr
    password: str


class AuthResponse(BaseModel):
    access_token: str
    refresh_token: str
    user_id: str
    email: str


class RefreshRequest(BaseModel):
    refresh_token: str


class UserResponse(BaseModel):
    user_id: str
    email: str


@router.post("/register", response_model=AuthResponse, status_code=status.HTTP_201_CREATED)
async def register(request: RegisterRequest):
    supabase = get_supabase()
    try:
        response = supabase.auth.sign_up({
            "email": request.email,
            "password": request.password,
        })
    except Exception as e:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=str(e))

    if response.user is None or response.session is None:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Registration failed. Check if email confirmation is required in Supabase settings.",
        )

    return AuthResponse(
        access_token=response.session.access_token,
        refresh_token=response.session.refresh_token,
        user_id=str(response.user.id),
        email=response.user.email,
    )


@router.post("/login", response_model=AuthResponse)
async def login(request: LoginRequest):
    supabase = get_supabase()
    try:
        response = supabase.auth.sign_in_with_password({
            "email": request.email,
            "password": request.password,
        })
    except Exception as e:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail=str(e))

    if response.user is None or response.session is None:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid credentials")

    return AuthResponse(
        access_token=response.session.access_token,
        refresh_token=response.session.refresh_token,
        user_id=str(response.user.id),
        email=response.user.email,
    )


@router.post("/refresh", response_model=AuthResponse)
async def refresh_token(request: RefreshRequest):
    supabase = get_supabase()
    try:
        response = supabase.auth.refresh_session(request.refresh_token)
    except Exception as e:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail=str(e))

    if response.user is None or response.session is None:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid refresh token")

    return AuthResponse(
        access_token=response.session.access_token,
        refresh_token=response.session.refresh_token,
        user_id=str(response.user.id),
        email=response.user.email,
    )


@router.post("/logout", status_code=status.HTTP_204_NO_CONTENT)
async def logout(current_user: User = Depends(get_current_user)):
    supabase = get_supabase()
    try:
        supabase.auth.sign_out()
    except Exception:
        pass


@router.get("/me", response_model=UserResponse)
async def me(current_user: User = Depends(get_current_user)):
    return UserResponse(
        user_id=str(current_user.id),
        email=current_user.email,
    )
