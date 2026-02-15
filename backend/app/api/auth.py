from datetime import datetime
from fastapi import APIRouter, HTTPException, Depends, status
from fastapi.security import OAuth2PasswordRequestForm
from app.core.database import get_database
from app.schemas.schemas import UserCreate, User, Token
from app.core import security
import logging

router = APIRouter()

@router.post("/register", response_model=User)
async def register(user: UserCreate, db=Depends(get_database)):
    """
    Register a new user with email and password.
    """
    existing_user = await db.users.find_one({"email": user.email})
    if existing_user:
        raise HTTPException(
            status_code=400,
            detail="A user with this email already exists."
        )

    hashed_password = security.get_password_hash(user.password)
    new_user_doc = {
        "email": user.email,
        "full_name": user.full_name,
        "password_hash": hashed_password,
        "created_at": datetime.now(),
        "is_active": True
    }
    
    result = await db.users.insert_one(new_user_doc)
    
    # Return user object matching Pydantic schema
    created_user = {
        "id": str(result.inserted_id),
        "email": user.email,
        "full_name": user.full_name,
        "is_active": True
    }
    
    return created_user

@router.post("/token", response_model=Token)
async def login_for_access_token(form_data: OAuth2PasswordRequestForm = Depends(), db=Depends(get_database)):
    """
    OAuth2 compatible token login, retrieve an access token for future requests.
    """
    user = await db.users.find_one({"email": form_data.username})
    if not user:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Incorrect username or password",
            headers={"WWW-Authenticate": "Bearer"},
        )
    
    if not security.verify_password(form_data.password, user["password_hash"]):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Incorrect username or password",
            headers={"WWW-Authenticate": "Bearer"},
        )

    access_token = security.create_access_token(
        subject=user["email"]
    )
    return {"access_token": access_token, "token_type": "bearer"}
