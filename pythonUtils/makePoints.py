import pandas as pd
import requests
import time
import random

STOP_TYPE = {
    1: (15, 40), 
    2: (7, 15), 
    3: (1, 6)   
}

OSRM_DELAY = 0.2

points = pd.read_csv("points.csv")

passageiros_list = [
    random.randint(*STOP_TYPE[row.stop_type])
    for _, row in points.iterrows()
]
points["passageiros"] = passageiros_list

points[["id", "endereço", "lat", "lon", "passageiros"]].to_csv("vertices.csv", index=False)

arestas = []

for i in range(len(points)):
    for j in range(len(points)):
        if i == j:
            continue
        lat1, lon1 = points.loc[i, "lat"], points.loc[i, "lon"]
        lat2, lon2 = points.loc[j, "lat"], points.loc[j, "lon"]

        if lat1 == 0 or lat2 == 0:
            tempo_min = 9999 
        else:
            url = f"http://router.project-osrm.org/route/v1/driving/{lon1},{lat1};{lon2},{lat2}?overview=false"
            
            try:
                r = requests.get(url).json()
                
                if "routes" in r and len(r["routes"]) > 0:
                    tempo_min = round(r["routes"][0]["duration"] / 60, 2)
                else:
                    tempo_min = 9999  
            except Exception as e:
                tempo_min = 9999  
            time.sleep(OSRM_DELAY)

        passageiros_dest = points.loc[j, "passageiros"]
        arestas.append([points.loc[i, "id"], points.loc[j, "id"], tempo_min, passageiros_dest])

df_edges = pd.DataFrame(arestas, columns=["origem", "destino", "tempo_min", "passageiros"])

df_edges.to_csv("arestas.csv", index=False)
