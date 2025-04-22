import openmeteo_requests
import requests_cache
import pandas as pd
from retry_requests import retry

class WeatherDataClient:
    def __init__(self, latitude, longitude):
        # Setup the Open-Meteo API client with cache and retry on error
        cache_session = requests_cache.CachedSession('.cache', expire_after=3600)
        retry_session = retry(cache_session, retries=5, backoff_factor=0.2)
        self.openmeteo = openmeteo_requests.Client(session=retry_session)
        self.latitude = latitude
        self.longitude = longitude

    def fetch_current_weather(self):
        url = "https://api.open-meteo.com/v1/forecast"
        params = {
            "latitude": self.latitude,
            "longitude": self.longitude,
            "current_weather": True,
            "timezone": "auto"
        }
        response = self.openmeteo.weather_api(url, params=params)[0]
        current = response.Current()
        # Extract data
        current_temperature = current.Variables(0).Value()
        current_cloud_cover = current.Variables(1).Value()
        return {
            "temperature": current_temperature,
            "cloud_cover": current_cloud_cover
        }