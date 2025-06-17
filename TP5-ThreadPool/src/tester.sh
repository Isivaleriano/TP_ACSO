#!/usr/bin/env bash

# Configuración
TARGET="custom"
MEMCHECK_LOG="valgrind_memcheck.log"
HELGRIND_LOG="valgrind_helgrind.log"

# Colores
GREEN='\033[1;32m'
RED='\033[1;31m'
YELLOW='\033[1;33m'
RESET='\033[0m'

# Abort con limpieza
abort() {
  local log="$1"
  echo -e "${RED}❌ Error detectado. Ver detalles en $log${RESET}"
  [[ "$log" != "$MEMCHECK_LOG" ]] && rm -f "$MEMCHECK_LOG"
  [[ "$log" != "$HELGRIND_LOG" ]] && rm -f "$HELGRIND_LOG"
  make clean > /dev/null 2>&1
  exit 1
}

# Paso 1: limpieza
echo -e "${YELLOW}🧹 Limpieza inicial...${RESET}"
make clean > /dev/null 2>&1
rm -f "$MEMCHECK_LOG" "$HELGRIND_LOG"

# Paso 2: compilación
echo -e "\n${YELLOW}🔧 Compilando ($TARGET)...${RESET}"
make "$TARGET" || abort ""

# Paso 3: tests funcionales
echo -e "\n${YELLOW}🧪 Ejecutando pruebas funcionales...${RESET}"
./threadpool --all || abort ""

# Paso 4: Valgrind Memcheck
if command -v valgrind &> /dev/null; then
  echo -e "\n${YELLOW}🔍 Valgrind Memcheck...${RESET}"
  valgrind --tool=memcheck --leak-check=full \
           ./threadpool --all &> "$MEMCHECK_LOG"

  if grep -qE '==.*ERROR SUMMARY: [1-9]' "$MEMCHECK_LOG" && \
     grep -qE 'definitely lost: [1-9][0-9]* bytes' "$MEMCHECK_LOG"; then
    abort "$MEMCHECK_LOG"
  else
    echo -e "${GREEN}✅ Memcheck OK: sin pérdidas ni errores críticos${RESET}"
  fi
else
  echo -e "${YELLOW}⚠️  Omitiendo Memcheck: valgrind no está instalado${RESET}"
fi

# Paso 5: Helgrind (race detection)
if command -v valgrind &> /dev/null; then
  echo -e "\n${YELLOW}🧵 Valgrind Helgrind (race detection)...${RESET}"
  valgrind --tool=helgrind --error-exitcode=1 \
           ./threadpool --all &> "$HELGRIND_LOG"

  if [[ $? -ne 0 ]]; then
    abort "$HELGRIND_LOG"
  else
    echo -e "${GREEN}✅ Helgrind OK: no se detectaron data races${RESET}"
  fi
else
  echo -e "${YELLOW}⚠️  Omitiendo Helgrind: valgrind no está instalado${RESET}"
fi

# Paso 6: limpieza final
echo -e "\n${YELLOW}🧹 Limpieza final...${RESET}"
make clean > /dev/null 2>&1
rm -f "$MEMCHECK_LOG" "$HELGRIND_LOG"

echo -e "\n${GREEN}🎯 Todos los pasos fueron completados correctamente${RESET}"
