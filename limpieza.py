"""
limpieza.py
Limpieza de datos — Red Vial Bolivia (OpenStreetMap/Geofabrik)
Proyecto: Rutas Óptimas en Red Vial Urbana
"""

import pandas as pd

# ──────────────────────────────────────────────
# 0. CARGA
# ──────────────────────────────────────────────
edges = pd.read_csv("edges.csv")
nodes = pd.read_csv("nodes.csv")

total_edges_orig = len(edges)
total_nodes_orig = len(nodes)

print("=" * 55)
print("REPORTE DE LIMPIEZA — RED VIAL BOLIVIA")
print("=" * 55)
print(f"\n[ANTES] Nodos: {total_nodes_orig:,}  |  Aristas: {total_edges_orig:,}\n")

# ──────────────────────────────────────────────
# 1. EDGES — self-loops (from_id == to_id)
#    Un segmento que empieza y termina en el mismo
#    nodo no aporta conectividad al grafo.
# ──────────────────────────────────────────────
self_loops = (edges["from_id"] == edges["to_id"]).sum()
edges = edges[edges["from_id"] != edges["to_id"]].copy()
print(f"[1] Self-loops eliminados          : {self_loops:>8,}")

# ──────────────────────────────────────────────
# 2. EDGES — distancia inválida (≤ 0)
# ──────────────────────────────────────────────
bad_dist = (edges["distance_m"] <= 0).sum()
edges = edges[edges["distance_m"] > 0].copy()
print(f"[2] Aristas con distance_m ≤ 0     : {bad_dist:>8,}")

# ──────────────────────────────────────────────
# 3. EDGES — duplicados por par (from_id, to_id)
#    Cuando dos OSM-ways conectan los mismos nodos,
#    conservamos el de menor distancia (más corto).
# ──────────────────────────────────────────────
before_dup = len(edges)
edges = edges.sort_values("distance_m").drop_duplicates(
    subset=["from_id", "to_id"], keep="first"
).reset_index(drop=True)
dup_removed = before_dup - len(edges)
print(f"[3] Duplicados (from,to) eliminados: {dup_removed:>8,}")

# ──────────────────────────────────────────────
# 4. EDGES — normalizar oneway
#    El campo puede venir como 0/1, true/false, yes/no.
#    Lo convertimos a int 0/1.
# ──────────────────────────────────────────────
def normalizar_oneway(val):
    s = str(val).strip().lower()
    if s in ("1", "true", "yes", "t"):
        return 1
    return 0

edges["oneway"] = edges["oneway"].apply(normalizar_oneway)
print(f"[4] oneway normalizado a 0/1       : ok  "
      f"(oneway=1: {edges['oneway'].sum():,})")

# ──────────────────────────────────────────────
# 5. EDGES — normalizar maxspeed
#    Acepta: 50, "50 km/h", 0 (sin dato), vacío.
#    0 lo tratamos como "sin dato" y lo reemplazamos
#    con la velocidad estándar por tipo de vía.
# ──────────────────────────────────────────────
VELOCIDAD_POR_FCLASS = {
    "motorway":       100, "motorway_link":   80,
    "trunk":           80, "trunk_link":       60,
    "primary":         60, "primary_link":     50,
    "secondary":       50, "secondary_link":   40,
    "tertiary":        40, "tertiary_link":    30,
    "residential":     30, "living_street":    20,
    "service":         20, "unclassified":     30,
    "track":           20, "track_grade1":     30,
    "track_grade2":    25, "track_grade3":     20,
    "track_grade4":    15, "track_grade5":     10,
    "footway":         5,  "path":              5,
    "steps":           3,  "pedestrian":        5,
    "cycleway":        15, "bridleway":        10,
    "busway":          40, "unknown":          30,
}

def normalizar_maxspeed(row):
    """Extrae km/h numérico; si es 0 o vacío usa tabla de referencia."""
    val = str(row["maxspeed"]).strip().lower()
    try:
        speed = float(val.replace("km/h", "").replace("mph", "").strip())
        if speed <= 0:
            raise ValueError
        return int(speed)
    except ValueError:
        return VELOCIDAD_POR_FCLASS.get(str(row["fclass"]).strip(), 30)

sin_maxspeed = (edges["maxspeed"] == 0).sum()
edges["maxspeed"] = edges.apply(normalizar_maxspeed, axis=1)
print(f"[5] maxspeed imputados (eran 0)    : {sin_maxspeed:>8,}")

# ──────────────────────────────────────────────
# 6. EDGES — calcular tiempo de recorrido (segundos)
#    t = (distance_m / 1000) / speed_kmh * 3600
# ──────────────────────────────────────────────
edges["time_s"] = (edges["distance_m"] / 1000) / edges["maxspeed"] * 3600
edges["time_s"] = edges["time_s"].round(2)
print(f"[6] Columna time_s calculada       : ok")

# ──────────────────────────────────────────────
# 7. NODES — eliminar nodos huérfanos
#    Nodos que no aparecen en ninguna arista limpia.
# ──────────────────────────────────────────────
nodos_en_aristas = set(edges["from_id"]).union(set(edges["to_id"]))
before_nodes = len(nodes)
nodes = nodes[nodes["node_id"].isin(nodos_en_aristas)].reset_index(drop=True)
huerfanos = before_nodes - len(nodes)
print(f"[7] Nodos huérfanos eliminados     : {huerfanos:>8,}")

# ──────────────────────────────────────────────
# 8. NODOS — coordenadas fuera de Bolivia
#    EPSG:32719 — Bolivia está aprox. en:
#    lat (northing): 7_300_000 – 9_000_000
#    lon (easting):  300_000  – 1_800_000
# ──────────────────────────────────────────────
mask_invalido = (
    (nodes["lat"] < 7_300_000) | (nodes["lat"] > 9_100_000) |
    (nodes["lon"] < 250_000)   | (nodes["lon"] > 1_850_000)
)
nodos_invalidos = mask_invalido.sum()
nodos_invalidos_ids = set(nodes.loc[mask_invalido, "node_id"])
nodes = nodes[~mask_invalido].reset_index(drop=True)
# También eliminamos aristas que referencien esos nodos
aristas_con_nodo_invalido = edges[
    edges["from_id"].isin(nodos_invalidos_ids) |
    edges["to_id"].isin(nodos_invalidos_ids)
].shape[0]
edges = edges[
    ~edges["from_id"].isin(nodos_invalidos_ids) &
    ~edges["to_id"].isin(nodos_invalidos_ids)
].reset_index(drop=True)
print(f"[8] Nodos fuera de Bolivia         : {nodos_invalidos:>8,}")
print(f"    Aristas eliminadas por esto    : {aristas_con_nodo_invalido:>8,}")

# ──────────────────────────────────────────────
# RESUMEN FINAL
# ──────────────────────────────────────────────
print()
print("=" * 55)
print("RESUMEN")
print("=" * 55)
print(f"  Nodos  antes : {total_nodes_orig:>10,}")
print(f"  Nodos  después: {len(nodes):>10,}  "
      f"({total_nodes_orig - len(nodes):,} eliminados, "
      f"{(total_nodes_orig - len(nodes)) / total_nodes_orig * 100:.1f}%)")
print(f"  Aristas antes : {total_edges_orig:>10,}")
print(f"  Aristas después:{len(edges):>10,}  "
      f"({total_edges_orig - len(edges):,} eliminadas, "
      f"{(total_edges_orig - len(edges)) / total_edges_orig * 100:.1f}%)")
print()
print(f"  Columnas edges limpias: {list(edges.columns)}")
print(f"  fclass más frecuentes:\n{edges['fclass'].value_counts().head(5).to_string()}")
print(f"\n  maxspeed promedio por fclass (muestra):")
print(edges.groupby("fclass")["maxspeed"].mean().sort_values(ascending=False)
      .head(8).round(1).to_string())
print()

# ──────────────────────────────────────────────
# GUARDAR
# ──────────────────────────────────────────────
edges.to_csv("edges_clean.csv", index=False)
nodes.to_csv("nodes_clean.csv", index=False)
print("Archivos guardados: edges_clean.csv  |  nodes_clean.csv")




