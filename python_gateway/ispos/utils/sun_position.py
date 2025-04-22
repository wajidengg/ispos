from anyio import current_time
import pandas as pd
from datetime import datetime, timezone
import pytz
import pvlib

from ispos.config import LATITUDE, LONGITUDE

# Function to determine sun's position
def get_sun_position(latitude, longitude, timestamp):
    # Define the location and time
    current_time_utc = datetime.now(timezone.utc).isoformat()
    times = pd.date_range(current_time_utc, periods=1, freq="H", tz="UTC")

    # Calculate solar position using PVLib
    solar_position = pvlib.solarposition.get_solarposition(times, LATITUDE, LONGITUDE)

    # Extract azimuth and altitude
    azimuth = solar_position["azimuth"].iloc[0]
    altitude = solar_position["elevation"].iloc[0]

    
    # Determine if the sun is above or below the horizon
    if altitude > 0:
        horizon_status = "above horizon"
        # Determine the azimuth status (east, middle, or west)
        if azimuth < 120:
            azimuth_status = "east"
        elif azimuth > 250:
            azimuth_status = "west"
        else:
            azimuth_status = "middle"
    else:
        horizon_status = "below horizon"
        azimuth_status = "none"
    
    # Output the results
    return horizon_status, azimuth_status, altitude, azimuth

