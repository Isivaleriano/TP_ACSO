#!/usr/bin/env bash
set -e

# === Configuración de rutas ===
TEST_DIR=samples/testdisks
MY_OUT=outputs/my
GOLD_OUT=outputs/gold

# === Crear carpetas de salida ===
mkdir -p $MY_OUT/ip $GOLD_OUT/ip

# === Limpiar salidas anteriores ===
rm -f $MY_OUT/ip/*.txt $GOLD_OUT/ip/*.txt

# === Detectar imágenes de disco (sin los .gold) ===
IMAGES=($(ls $TEST_DIR | grep -v '\.gold$'))

echo "🔍 Ejecutando pruebas con las siguientes imágenes:"
printf ' - %s\n' "${IMAGES[@]}"

# === Ejecutar pruebas -ip (inode + pathname) ===
echo "▶ Ejecutando pruebas -ip..."
for img in "${IMAGES[@]}"; do
  ./diskimageaccess -ip $TEST_DIR/$img > $MY_OUT/ip/${img}.txt
  ./samples/diskimageaccess_soln_x86 -ip $TEST_DIR/$img > $GOLD_OUT/ip/${img}.txt
done

# === Comparación automática con diff ===
echo "🔍 Comparando salidas con la cátedra..."

DIFF_FOUND=0

for file in "$MY_OUT/ip"/*.txt; do
  fname=$(basename "$file")
  gold_file="$GOLD_OUT/ip/$fname"
  if [ ! -f "$gold_file" ]; then
    echo "❌ Falta archivo de referencia: $gold_file"
    DIFF_FOUND=1
  elif ! diff -q "$file" "$gold_file" > /dev/null; then
    echo "❌ Diferencia encontrada: ip/$fname"
    DIFF_FOUND=1
  fi
done

# === Resultado final ===
if [ "$DIFF_FOUND" -eq 0 ]; then
  echo "✅ Todas las salidas coinciden con las de la cátedra."
else
  echo "⚠️ Algunas salidas no coinciden. Revisá los archivos marcados."
fi
