"""
=============================================================================
EXPLORACIÓN Y LIMPIEZA PREVIA — Red Vial Bolivia (OpenStreetMap/Geofabrik)
=============================================================================
Proyecto: Rutas Óptimas en Red Vial Urbana
Dataset:  nodes.csv  (886 790 nodos)   |   edges.csv  (588 485 aristas)
CRS:      EPSG:32719 — UTM Zona 19S (unidades en metros)

Estructura de archivos
----------------------
  nodes.csv : node_id, lat (Northing, m), lon (Easting, m)
  edges.csv : osm_id, from_id, to_id, distance_m, fclass, oneway, maxspeed

Nota sobre los nombres de columnas
-----------------------------------
Aunque el CSV usa 'lat' y 'lon', los valores son coordenadas UTM proyectadas
(no grados decimales). 'lat' corresponde a Northing y 'lon' a Easting.
Esto es coherente con el script main.py del proyecto que proyecta a EPSG:32719.
=============================================================================
"""

import pandas as pd
import numpy as np
import time

# ─────────────────────────────────────────────────────────────────────────────
# CONFIGURACIÓN
# ─────────────────────────────────────────────────────────────────────────────

NODES_PATH = "nodes.csv"   # Ajustar ruta si es necesario
EDGES_PATH = "edges.csv"

# Umbral de distancia máxima razonable para un segmento de calle (en metros).
# Un valor >100 km es sospechoso para un segmento individual de OSM.
MAX_SEGMENT_DISTANCE_M = 100_000

# Clases de vía que NO son vehiculares y que pueden excluirse en análisis
# de rutas en automóvil. Se marcan, no se eliminan automáticamente, porque
# para análisis peatonales o de emergencia podrían ser relevantes.
NON_VEHICLE_FCLASSES = {
    "footway", "path", "steps", "cycleway",
    "pedestrian", "track_grade4", "track_grade5",
}

# Velocidades supuestas por tipo de vía (km/h) cuando maxspeed == 0.
# Fuente: estándares OSM latinoamericanos y normativa vial boliviana aproximada.
SPEED_BY_FCLASS = {
    "motorway":       100,
    "motorway_link":   80,
    "trunk":           80,
    "trunk_link":      60,
    "primary":         60,
    "primary_link":    50,
    "secondary":       50,
    "secondary_link":  40,
    "tertiary":        40,
    "tertiary_link":   30,
    "residential":     30,
    "living_street":   20,
    "service":         20,
    "unclassified":    30,
    "track":           20,
    "track_grade2":    20,
    "track_grade3":    15,
    "track_grade4":    10,
    "track_grade5":    10,
    "path":            10,
    "footway":          5,
    "steps":            3,
    "cycleway":        15,
    "pedestrian":       5,
}
DEFAULT_SPEED = 20  # km/h para fclasses desconocidas

separator = "=" * 72


def section(title):
    print(f"\n{separator}")
    print(f"  {title}")
    print(separator)


# ─────────────────────────────────────────────────────────────────────────────
# 1. CARGA DE DATOS
# ─────────────────────────────────────────────────────────────────────────────

section("1. CARGA DE DATOS")

t0 = time.time()
nodes = pd.read_csv(NODES_PATH)
edges = pd.read_csv(EDGES_PATH)
t1 = time.time()

print(f"  nodes.csv cargado: {len(nodes):>10,} filas  |  columnas: {list(nodes.columns)}")
print(f"  edges.csv cargado: {len(edges):>10,} filas  |  columnas: {list(edges.columns)}")
print(f"  Tiempo de carga: {t1 - t0:.1f}s")

# Registros originales para el reporte final
original_nodes = len(nodes)
original_edges = len(edges)


# ─────────────────────────────────────────────────────────────────────────────
# 2. EXPLORACIÓN INICIAL — ESTRUCTURA Y TIPOS
# ─────────────────────────────────────────────────────────────────────────────

section("2. EXPLORACIÓN INICIAL")

print("\n── nodes.csv ──")
print(nodes.dtypes.to_string())
print(nodes.describe().to_string())

print("\n── edges.csv ──")
print(edges.dtypes.to_string())
print(edges.describe().to_string())

print("\n── Distribución de fclass (edges) ──")
print(edges["fclass"].value_counts().to_string())

print("\n── Valores únicos de oneway ──", edges["oneway"].unique())
print("── Valores únicos de maxspeed (primeros 20) ──",
      sorted(edges["maxspeed"].unique())[:20])


# ─────────────────────────────────────────────────────────────────────────────
# 3. DETECCIÓN DE PROBLEMAS — NODOS
# ─────────────────────────────────────────────────────────────────────────────

section("3. DETECCIÓN DE PROBLEMAS — NODOS")

issues_nodes = {}

# 3a. Nulos
null_nodes = nodes.isnull().sum()
if null_nodes.any():
    issues_nodes["nulos"] = null_nodes[null_nodes > 0].to_dict()
    print(f"  [!] Valores nulos encontrados: {issues_nodes['nulos']}")
else:
    print("  [OK] Sin valores nulos en nodes.csv")

# 3b. Duplicados en node_id
dup_id = nodes.duplicated(subset=["node_id"]).sum()
issues_nodes["node_id_duplicados"] = dup_id
print(f"  [{'!' if dup_id else 'OK'}] node_id duplicados: {dup_id}")

# 3c. Coordenadas duplicadas (nodos superpuestos)
dup_coords = nodes.duplicated(subset=["lat", "lon"]).sum()
issues_nodes["coords_duplicadas"] = dup_coords
print(f"  [{'!' if dup_coords else 'OK'}] Coordenadas (lat,lon) duplicadas: {dup_coords}")

# 3d. Coordenadas fuera de rango UTM 19S para Bolivia
#     Northing Bolivia aprox: 7 400 000 – 8 900 000 m
#     Easting  Bolivia aprox:   400 000 – 1 700 000 m
UTM_N_MIN, UTM_N_MAX = 7_400_000, 8_900_000
UTM_E_MIN, UTM_E_MAX =   400_000, 1_700_000
out_lat = nodes[(nodes["lat"] < UTM_N_MIN) | (nodes["lat"] > UTM_N_MAX)]
out_lon = nodes[(nodes["lon"] < UTM_E_MIN) | (nodes["lon"] > UTM_E_MAX)]
issues_nodes["coords_fuera_rango_lat"] = len(out_lat)
issues_nodes["coords_fuera_rango_lon"] = len(out_lon)
print(f"  [{'!' if len(out_lat) else 'OK'}] Nodos con Northing fuera de Bolivia: {len(out_lat)}")
print(f"  [{'!' if len(out_lon) else 'OK'}] Nodos con Easting fuera de Bolivia:  {len(out_lon)}")

# 3e. Nodos aislados (sin ninguna arista) — costoso en datasets grandes,
#     se evalúa solo si el grafo cabe cómodamente en memoria.
all_node_ids_in_edges = set(edges["from_id"]).union(set(edges["to_id"]))
isolated_nodes = (~nodes["node_id"].isin(all_node_ids_in_edges)).sum()
issues_nodes["nodos_aislados"] = isolated_nodes
print(f"  [{'!' if isolated_nodes else 'OK'}] Nodos sin ninguna arista (aislados): {isolated_nodes:,}")


# ─────────────────────────────────────────────────────────────────────────────
# 4. DETECCIÓN DE PROBLEMAS — ARISTAS
# ─────────────────────────────────────────────────────────────────────────────

section("4. DETECCIÓN DE PROBLEMAS — ARISTAS")

issues_edges = {}

# 4a. Nulos
null_edges = edges.isnull().sum()
if null_edges.any():
    issues_edges["nulos"] = null_edges[null_edges > 0].to_dict()
    print(f"  [!] Valores nulos encontrados: {issues_edges['nulos']}")
else:
    print("  [OK] Sin valores nulos en edges.csv")

# 4b. Self-loops (from_id == to_id)
# Un segmento que empieza y termina en el mismo nodo no aporta información
# de conectividad y puede distorsionar algoritmos de camino más corto.
self_loops_mask = edges["from_id"] == edges["to_id"]
n_self_loops = self_loops_mask.sum()
issues_edges["self_loops"] = n_self_loops
print(f"  [{'!' if n_self_loops else 'OK'}] Self-loops (from_id == to_id): {n_self_loops:,}")

# 4c. Aristas dirigidas duplicadas (mismo from→to con mismo osm_id base)
dup_directed_mask = edges.duplicated(subset=["from_id", "to_id"], keep=False)
n_dup_directed = dup_directed_mask.sum()
issues_edges["duplicados_dirigidos"] = n_dup_directed
print(f"  [{'!' if n_dup_directed else 'OK'}] Aristas dirigidas duplicadas (from,to): {n_dup_directed:,}")

# 4d. Aristas no dirigidas duplicadas (A→B y B→A para mismo par de nodos)
# En un grafo no dirigido esto sería redundancia. En uno dirigido puede ser
# válido (ida y vuelta). Se reporta para decisión del modelador.
edges_temp = edges.copy()
edges_temp["u"] = edges_temp[["from_id", "to_id"]].min(axis=1)
edges_temp["v"] = edges_temp[["from_id", "to_id"]].max(axis=1)
dup_undirected_mask = edges_temp.duplicated(subset=["u", "v"], keep=False)
n_dup_undir = dup_undirected_mask.sum()
issues_edges["duplicados_no_dirigidos"] = n_dup_undir
print(f"  [i] Pares de nodos con aristas en ambas direcciones (A↔B): {n_dup_undir:,}")
print(f"      → En un grafo DIRIGIDO esto puede ser válido (calle de doble vía).")
print(f"        Sólo se eliminarían si oneway=0 y la distancia es idéntica.")

# 4e. Distancias inválidas
dist_zero_mask = edges["distance_m"] <= 0
dist_huge_mask = edges["distance_m"] > MAX_SEGMENT_DISTANCE_M
n_dist_zero = dist_zero_mask.sum()
n_dist_huge = dist_huge_mask.sum()
issues_edges["distance_lte_0"] = n_dist_zero
issues_edges["distance_gt_100km"] = n_dist_huge
print(f"  [{'!' if n_dist_zero else 'OK'}] Aristas con distance_m <= 0: {n_dist_zero:,}")
print(f"  [{'!' if n_dist_huge else 'OK'}] Aristas con distance_m > {MAX_SEGMENT_DISTANCE_M/1000:.0f} km "
      f"(sospechosas): {n_dist_huge:,}")
if n_dist_huge > 0:
    print("      Valores afectados (muestra):")
    print(edges[dist_huge_mask][["osm_id", "from_id", "to_id", "distance_m", "fclass"]]
          .head(5).to_string(index=False))

# 4f. maxspeed == 0 (valor ausente enmascarado como 0)
# El script de extracción usa 0 cuando no hay maxspeed declarado.
# Esto afecta al 89% de las aristas y es crítico para calcular tiempo estimado.
maxspeed_zero_mask = edges["maxspeed"] == 0
n_maxspeed_zero = maxspeed_zero_mask.sum()
pct_maxspeed = 100 * n_maxspeed_zero / len(edges)
issues_edges["maxspeed_cero_o_ausente"] = n_maxspeed_zero
print(f"\n  [!] maxspeed == 0 (velocidad no registrada): "
      f"{n_maxspeed_zero:,} ({pct_maxspeed:.1f}% del total)")
print(f"      → Crítico para cálculo de tiempo estimado.")
print(f"        Se imputará usando velocidad supuesta por fclass.")

# 4g. Nodos referenciados en aristas que no existen en nodes.csv
all_node_ids = set(nodes["node_id"])
missing_from_mask = ~edges["from_id"].isin(all_node_ids)
missing_to_mask   = ~edges["to_id"].isin(all_node_ids)
n_miss_from = missing_from_mask.sum()
n_miss_to   = missing_to_mask.sum()
issues_edges["from_id_no_existe"] = n_miss_from
issues_edges["to_id_no_existe"]   = n_miss_to
print(f"\n  [{'!' if n_miss_from else 'OK'}] Aristas con from_id sin nodo: {n_miss_from:,}")
print(f"  [{'!' if n_miss_to   else 'OK'}] Aristas con to_id sin nodo:   {n_miss_to:,}")

# 4h. Vías no vehiculares
non_vehicle_mask = edges["fclass"].isin(NON_VEHICLE_FCLASSES)
n_non_vehicle = non_vehicle_mask.sum()
issues_edges["aristas_no_vehiculares"] = n_non_vehicle
print(f"\n  [i] Aristas de tipo no vehicular (peatonal/ciclista/sendero): "
      f"{n_non_vehicle:,} ({100*n_non_vehicle/len(edges):.1f}%)")
print("      → Se marcan; no se eliminan automáticamente.")
print("      Detalle:")
print(edges[non_vehicle_mask]["fclass"].value_counts().to_string())


# ─────────────────────────────────────────────────────────────────────────────
# 5. LIMPIEZAS JUSTIFICADAS
# ─────────────────────────────────────────────────────────────────────────────

section("5. APLICACIÓN DE LIMPIEZAS JUSTIFICADAS")

nodes_clean = nodes.copy()
edges_clean = edges.copy()

report_actions = []

# ── 5a. Eliminar self-loops ──────────────────────────────────────────────────
# JUSTIFICACIÓN: Un segmento donde from_id == to_id no representa un tramo
# de calle real y no aporta conectividad. En Dijkstra generaría un ciclo de
# coste 0 o positivo que puede afectar el conteo de alcance vehicular.
before = len(edges_clean)
edges_clean = edges_clean[edges_clean["from_id"] != edges_clean["to_id"]]
removed = before - len(edges_clean)
report_actions.append(
    f"  ✓ Self-loops eliminados: {removed:,} aristas "
    f"(from_id == to_id, no representan tramos reales)"
)
print(f"  [5a] Self-loops eliminados: {removed:,}")

# ── 5b. Eliminar aristas dirigidas estrictamente duplicadas ──────────────────
# JUSTIFICACIÓN: Dos aristas con idéntico (from_id, to_id) y distancia similar
# son un artefacto de extracción (por ejemplo, el mismo segmento OSM procesado
# dos veces). Se conserva la de menor distancia como valor más conservador.
before = len(edges_clean)
edges_clean = edges_clean.sort_values("distance_m")
edges_clean = edges_clean.drop_duplicates(subset=["from_id", "to_id"], keep="first")
removed = before - len(edges_clean)
report_actions.append(
    f"  ✓ Aristas dirigidas duplicadas eliminadas: {removed:,} "
    f"(se conservó la de menor distance_m)"
)
print(f"  [5b] Aristas dirigidas duplicadas eliminadas: {removed:,}")

# ── 5c. Marcar (no eliminar) aristas con distancia sospechosamente grande ────
# JUSTIFICACIÓN: Segmentos >100 km pueden ser errores de geometría en OSM
# (nodos extremadamente alejados mal conectados). Se marcan con una columna
# de bandera para que el analista los revise antes de incluirlos.
edges_clean["flag_dist_sospechosa"] = edges_clean["distance_m"] > MAX_SEGMENT_DISTANCE_M
n_flagged = edges_clean["flag_dist_sospechosa"].sum()
report_actions.append(
    f"  ✓ Aristas con distancia > {MAX_SEGMENT_DISTANCE_M/1000:.0f} km marcadas "
    f"(flag_dist_sospechosa): {n_flagged:,} — no eliminadas, requieren revisión manual"
)
print(f"  [5c] Aristas marcadas por distancia sospechosa (>100 km): {n_flagged:,}")

# ── 5d. Marcar aristas no vehiculares ────────────────────────────────────────
# JUSTIFICACIÓN: Para análisis de ruta vehicular, footway/steps/cycleway etc.
# son irrelevantes. Se marcan pero NO se eliminan porque el proyecto también
# podría requerir análisis peatonal o de emergencia con acceso a pie.
edges_clean["flag_no_vehicular"] = edges_clean["fclass"].isin(NON_VEHICLE_FCLASSES)
n_nv = edges_clean["flag_no_vehicular"].sum()
report_actions.append(
    f"  ✓ Aristas no vehiculares marcadas (flag_no_vehicular): {n_nv:,} — "
    f"no eliminadas; filtrar con edges_clean[~edges_clean['flag_no_vehicular']] "
    f"para análisis vehicular"
)
print(f"  [5d] Aristas no vehiculares marcadas: {n_nv:,}")

# ── 5e. Imputar maxspeed usando velocidad supuesta por fclass ─────────────────
# JUSTIFICACIÓN: El 89% de los registros tiene maxspeed=0. Para calcular
# el tiempo estimado (objetivo bonus del proyecto: distancia vs. tiempo)
# se necesita una velocidad. La imputación por fclass es la práctica estándar
# en redes OSM donde la velocidad no está declarada explícitamente.
# Se crea una columna nueva 'speed_kmh' para no alterar maxspeed original.
def impute_speed(row):
    if row["maxspeed"] > 0:
        return float(row["maxspeed"])
    return float(SPEED_BY_FCLASS.get(row["fclass"], DEFAULT_SPEED))

edges_clean["speed_kmh"] = edges_clean.apply(impute_speed, axis=1)
n_imputed = (edges["maxspeed"] == 0).sum()  # antes de limpieza, para referencia
report_actions.append(
    f"  ✓ Columna 'speed_kmh' creada: {n_imputed:,} velocidades imputadas por fclass; "
    f"maxspeed original no modificado"
)
print(f"  [5e] Columna 'speed_kmh' creada. {n_imputed:,} velocidades imputadas.")

# ── 5f. Calcular tiempo estimado de recorrido (travel_time_s) ────────────────
# JUSTIFICACIÓN: Objetivo explícito del proyecto — comparar ruta por distancia
# vs. por tiempo. travel_time_s = (distance_m / 1000) / speed_kmh * 3600
edges_clean["travel_time_s"] = (
    (edges_clean["distance_m"] / 1000.0) / edges_clean["speed_kmh"] * 3600.0
).round(2)
report_actions.append(
    "  ✓ Columna 'travel_time_s' creada: tiempo de recorrido en segundos "
    "(distance_m/1000 / speed_kmh * 3600)"
)
print("  [5f] Columna 'travel_time_s' creada (tiempo estimado en segundos).")

# ── 5g. Eliminar nodos con coordenadas fuera del rango Bolivia ───────────────
# JUSTIFICACIÓN: Nodos proyectados fuera del territorio boliviano son
# artefactos de reproyección o datos corruptos. Su presencia puede crear
# aristas con longitudes absurdas.
before_n = len(nodes_clean)
nodes_clean = nodes_clean[
    (nodes_clean["lat"].between(UTM_N_MIN, UTM_N_MAX)) &
    (nodes_clean["lon"].between(UTM_E_MIN, UTM_E_MAX))
]
removed_n = before_n - len(nodes_clean)
if removed_n > 0:
    # Eliminar también las aristas que referencian esos nodos
    valid_ids = set(nodes_clean["node_id"])
    before_e = len(edges_clean)
    edges_clean = edges_clean[
        edges_clean["from_id"].isin(valid_ids) &
        edges_clean["to_id"].isin(valid_ids)
    ]
    removed_e = before_e - len(edges_clean)
    report_actions.append(
        f"  ✓ Nodos fuera de Bolivia eliminados: {removed_n:,} nodos → "
        f"{removed_e:,} aristas huérfanas removidas en cascada"
    )
    print(f"  [5g] Nodos fuera de Bolivia: {removed_n:,} eliminados, "
          f"{removed_e:,} aristas en cascada.")
else:
    report_actions.append("  ✓ Sin nodos fuera del rango Bolivia — no se eliminó nada.")
    print("  [5g] Sin nodos fuera de Bolivia. No se realizaron cambios.")


# ─────────────────────────────────────────────────────────────────────────────
# 6. ESTADÍSTICAS FINALES DEL GRAFO LIMPIO
# ─────────────────────────────────────────────────────────────────────────────

section("6. ESTADÍSTICAS DEL GRAFO LIMPIO")

print(f"  Nodos:   {original_nodes:>10,}  →  {len(nodes_clean):>10,}  "
      f"(eliminados: {original_nodes - len(nodes_clean):,})")
print(f"  Aristas: {original_edges:>10,}  →  {len(edges_clean):>10,}  "
      f"(eliminadas: {original_edges - len(edges_clean):,})")

print(f"\n  distance_m  — media: {edges_clean['distance_m'].mean():.1f} m  "
      f"| mediana: {edges_clean['distance_m'].median():.1f} m  "
      f"| máx: {edges_clean['distance_m'].max():.1f} m")
print(f"  travel_time_s — media: {edges_clean['travel_time_s'].mean():.1f} s  "
      f"| mediana: {edges_clean['travel_time_s'].median():.1f} s")

print(f"\n  fclass tras limpieza:")
print(edges_clean["fclass"].value_counts().to_string())


# ─────────────────────────────────────────────────────────────────────────────
# 7. REPORTE FINAL EN CONSOLA
# ─────────────────────────────────────────────────────────────────────────────

section("7. REPORTE FINAL DE LIMPIEZA")

print("\n  PROBLEMAS DETECTADOS:")
print(f"    • Self-loops (aristas inútiles):          {issues_edges['self_loops']:>8,}")
print(f"    • Aristas dirigidas duplicadas:           {issues_edges['duplicados_dirigidos']:>8,}")
print(f"    • Aristas no dirigidas duplicadas (A↔B):  {issues_edges['duplicados_no_dirigidos']:>8,}  (INFO)")
print(f"    • Aristas con distancia > 100 km:         {issues_edges['distance_gt_100km']:>8,}")
print(f"    • maxspeed == 0 (sin velocidad):          {issues_edges['maxspeed_cero_o_ausente']:>8,}  ({pct_maxspeed:.1f}%)")
print(f"    • Aristas no vehiculares:                 {issues_edges['aristas_no_vehiculares']:>8,}  (14.3%)")
print(f"    • Nodos aislados:                         {issues_nodes['nodos_aislados']:>8,}")
print(f"    • Nodos con coords fuera de Bolivia:      {issues_nodes['coords_fuera_rango_lat'] + issues_nodes['coords_fuera_rango_lon']:>8,}")

print("\n  ACCIONES REALIZADAS:")
for action in report_actions:
    print(action)

print(f"\n  RESUMEN:")
print(f"    Nodos eliminados:   {original_nodes - len(nodes_clean):,}")
print(f"    Aristas eliminadas: {original_edges - len(edges_clean):,}")
print(f"    Columnas añadidas:  flag_dist_sospechosa, flag_no_vehicular, "
      f"speed_kmh, travel_time_s")


# ─────────────────────────────────────────────────────────────────────────────
# 8. EXPORTAR DATOS LIMPIOS
# ─────────────────────────────────────────────────────────────────────────────

section("8. EXPORTANDO DATOS LIMPIOS")

nodes_out = "nodes_clean.csv"
edges_out = "edges_clean.csv"

nodes_clean.to_csv(nodes_out, index=False)
edges_clean.to_csv(edges_out, index=False)

