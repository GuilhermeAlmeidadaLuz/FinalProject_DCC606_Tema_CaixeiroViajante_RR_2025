import pandas as pd
import folium
import requests

# Carrega os dados
vertices = pd.read_csv("points.csv")
arestas = pd.read_csv("arestas.csv")
rota = pd.read_csv("../Resultados/equilibrio.csv")["id"].tolist()

vertices = vertices[pd.to_numeric(vertices["lat"], errors="coerce").notnull()]
vertices = vertices[pd.to_numeric(vertices["lon"], errors="coerce").notnull()]
vertices["lat"] = vertices["lat"].astype(float)
vertices["lon"] = vertices["lon"].astype(float)

primeiro_ponto = vertices.loc[vertices["id"] == rota[0]].iloc[0]
m = folium.Map(location=[primeiro_ponto["lat"], primeiro_ponto["lon"]], zoom_start=14)

# Marca os pontos da rota
for pid in rota:
    if pid in vertices["id"].values:
        ponto = vertices.loc[vertices["id"] == pid].iloc[0]
        folium.Marker(
            location=[ponto["lat"], ponto["lon"]],
            popup=f"<b>{ponto['endereço']}</b>",
            icon=folium.Icon(color="blue", icon="bus", prefix="fa")
        ).add_to(m)

# Desenha as rotas
for i in range(len(rota) - 1):
    origem_id = rota[i]
    destino_id = rota[i + 1]

    if origem_id not in vertices["id"].values or destino_id not in vertices["id"].values:
        continue

    origem = vertices.loc[vertices["id"] == origem_id].iloc[0]
    destino = vertices.loc[vertices["id"] == destino_id].iloc[0]

    passageiros = arestas[
        (arestas["origem"] == origem_id) & (arestas["destino"] == destino_id)
    ]["passageiros"]

    passageiros_str = f"Passageiros: {passageiros.values[0]}" if not passageiros.empty else "Passageiros: N/D"

    url = (
        f"http://router.project-osrm.org/route/v1/driving/"
        f"{origem['lon']},{origem['lat']};{destino['lon']},{destino['lat']}"
        f"?overview=full&geometries=geojson"
    )

    try:
        r = requests.get(url).json()
        if "routes" in r and len(r["routes"]) > 0:
            coords = r["routes"][0]["geometry"]["coordinates"]
            coords = [(lat, lon) for lon, lat in coords]
        else:
            raise ValueError("No route found, using fallback.")
    except Exception as e:
        print(f"Erro ao calcular a rota entre {origem_id} e {destino_id}: {e}")
        coords = [(origem["lat"], origem["lon"]), (destino["lat"], destino["lon"])]

    folium.PolyLine(
        coords,
        color="red",
        weight=5,
        opacity=0.8,
        tooltip=passageiros_str
    ).add_to(m)

m.save("../Resultados/equilibrio.html")
