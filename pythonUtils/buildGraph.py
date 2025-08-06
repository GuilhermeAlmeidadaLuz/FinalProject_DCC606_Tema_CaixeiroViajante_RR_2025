import pandas as pd
import folium
import requests

vertices = pd.read_csv("vertices.csv")
rota = pd.read_csv("../tsp/tempo.csv")["id"].tolist()

#Renderiza o mapa
primeiro_ponto = vertices.loc[vertices["id"] == rota[0]].iloc[0]
m = folium.Map(location=[primeiro_ponto["lat"], primeiro_ponto["lon"]], zoom_start=14)

#Marca os pontos
for pid in rota:
    ponto = vertices.loc[vertices["id"] == pid].iloc[0]
    folium.Marker(
        location=[ponto["lat"], ponto["lon"]],
        popup=f"<b>{ponto['endereço']}</b><br>Passageiros: {ponto['passageiros']}",
        icon=folium.Icon(color="blue", icon="bus", prefix="fa")
    ).add_to(m)


#Traça as rotas
for i in range(len(rota) - 1):
    origem = vertices.loc[vertices["id"] == rota[i]].iloc[0]
    destino = vertices.loc[vertices["id"] == rota[i+1]].iloc[0]

    url = (
        f"http://router.project-osrm.org/route/v1/driving/"
        f"{origem['lon']},{origem['lat']};{destino['lon']},{destino['lat']}"
        f"?overview=full&geometries=geojson"
    )

    try:
        r = requests.get(url).json()
        coords = r["routes"][0]["geometry"]["coordinates"]
        coords = [(lat, lon) for lon, lat in coords]
        folium.PolyLine(coords, color="red", weight=5, opacity=0.8).add_to(m)
    except Exception as e:
        print("erro")

m.save("rota.html")
