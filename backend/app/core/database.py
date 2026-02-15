from motor.motor_asyncio import AsyncIOMotorClient
from app.core.config import settings
import logging

# We will initialize connection logic here
class Database:
    client: AsyncIOMotorClient = None
    db = None

    def connect(self):
        """Connect to the MongoDB database."""
        try:
            logging.info(f"Connecting to MongoDB at {settings.MONGODB_URL}")
            self.client = AsyncIOMotorClient(settings.MONGODB_URL)
            self.db = self.client[settings.DATABASE_NAME]
            logging.info("Connected to MongoDB")
        except Exception as e:
            logging.error(f"Failed to connect to MongoDB: {e}")
            raise e

    def close(self):
        """Close the MongoDB connection."""
        if self.client:
            self.client.close()
            logging.info("Closed MongoDB connection")

# Create a singleton instance
db_instance = Database()

def get_database():
    """Dependency injection helper to get the database."""
    return db_instance.db
