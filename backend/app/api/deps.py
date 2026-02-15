from typing import Generator, Optional
from fastapi import Depends, HTTPException, status, Header
from fastapi.security import OAuth2PasswordBearer
from jose import jwt, JWTError
from pydantic import ValidationError

from app.core import security
from app.core.config import settings
from app.core.database import get_database
from app.schemas.schemas import TokenData, User, Device

oauth2_scheme = OAuth2PasswordBearer(
    tokenUrl=f"{settings.API_V1_STR}/auth/token"
)

async def get_current_user(token: str = Depends(oauth2_scheme)) -> User:
    """
    Dependency to verify the JWT token and return the current user.
    """
    credentials_exception = HTTPException(
        status_code=status.HTTP_401_UNAUTHORIZED,
        detail="Could not validate credentials",
        headers={"WWW-Authenticate": "Bearer"},
    )
    try:
        payload = jwt.decode(token, settings.SECRET_KEY, algorithms=[settings.ALGORITHM])
        username: str = payload.get("sub")
        if username is None:
            raise credentials_exception
        token_data = TokenData(username=username)
    except (JWTError, ValidationError):
        raise credentials_exception

    db = get_database()
    user = await db.users.find_one({"email": token_data.username})
    if user is None:
        raise credentials_exception
    
    # Convert _id to string for Pydantic compatibility
    user_dict = {
        "id": str(user["_id"]),
        "email": user["email"],
        "full_name": user.get("full_name"),
        "is_active": user.get("is_active", True)
    }
    
    return User(**user_dict)

async def get_current_active_user(current_user: User = Depends(get_current_user)) -> User:
    """
    Verify that the user is active.
    """
    if not current_user.is_active:
        raise HTTPException(status_code=400, detail="Inactive user")
    return current_user

async def get_current_device(x_api_key: str = Header(...)) -> Device:
    """
    Verify device API Key.
    """
    db = get_database()
    device = await db.devices.find_one({"api_key": x_api_key})
    
    if not device:
        raise HTTPException(status_code=401, detail="Invalid API Key")
        
    device["id"] = str(device["_id"])
    return Device(**device)
