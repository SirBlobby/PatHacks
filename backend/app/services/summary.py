import google.generativeai as genai
from app.core.config import settings

# Configure Gemini API
if settings.GOOGLE_API_KEY:
    genai.configure(api_key=settings.GOOGLE_API_KEY)

async def generate_summary(text: str):
    """
    Generate a summary of the provided text using Gemini Pro.
    """
    if not settings.GOOGLE_API_KEY:
        return {
            "short_summary": "API Key missing. Unable to generate summary.",
            "detailed_summary": "API Key missing.",
            "key_points": []
        }
        
    model = genai.GenerativeModel('gemini-pro')
    
    # Simple prompt engineering
    prompt = f"""
    Analyze the following lecture transcript and provide:
    1. A short, one-sentence summary.
    2. A detailed summary (2-3 paragraphs).
    3. A list of key takeaways / bullet points.
    
    Transcript:
    {text[:30000]}  # Limit text to avoid token limits for now
    
    Format the output as JSON with keys: "short_summary", "detailed_summary", "key_points" (array of strings).
    """
    
    try:
        response = await model.generate_content_async(prompt)
        # Parse JSON from response
        # Gemini might return markdown JSON block, need to clean
        content = response.text
        # Clean markdown code blocks if present
        if "```json" in content:
            content = content.split("```json")[1].split("```")[0].strip()
        elif "```" in content:
            content = content.split("```")[1].split("```")[0].strip()
            
        import json
        return json.loads(content)
        
    except Exception as e:
        print(f"Error generating summary: {e}")
        return {
            "short_summary": "Error generating summary.",
            "detailed_summary": str(e),
            "key_points": []
        }
