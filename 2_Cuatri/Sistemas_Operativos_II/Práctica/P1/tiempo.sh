#!/bin/bash
# CÓDIGO DE MARCOS GARCÍA BLANCO Y NURIA GUERRA CASAL

# Comprobamos que se pasa una fecha
# $# es para comprobar el número de parámetros pasados al script y -ne indica "not equal"
if [ $# -ne 1 ]; then
    echo "Uso: ./tiempo.sh \"yyyy-mm-dd\""
    exit 1
fi

# Validar fechas inexistentes en 1582 por el cambio
# Comparar cadenas con > y <, esto funciona porque el formato YYYY-MM-DD es ordenable
# Comparamos con $1 porque almacena la variable de entrada 1 del script 
# Para fechas anteriores al 04-10-1582, el tiempo real incluye los 10 días que "desaparecieron", por lo que se siguen contando
if [[ "$1" > "1582-10-04" && "$1" < "1582-10-15" ]]; then
    echo "Fecha inválida, esos días no existieron."
    exit 1
fi

# Validar que el año no supere 2026, para ello se toman los 4 primeros caracteres del argumento $1 
#(${argumento : posicionInicial : numeroCaracteres})
ano_ingresado="${1:0:4}"
if [ "$ano_ingresado" -gt 2026 ]; then
    echo "Error, el año máximo permitido es 2026."
    exit 1
fi

# Obtener datos de la fecha de referencia
# date -d convierte la fecha a segundos desde 1970, y con +%Y, +%-j, etc. obtenemos año, día del año, etc.
# 2>/dev/null evita que date muestre errores si la fecha no es correcta (forzando que haga el if posterior)

ano_ref=$(date -d "$1" +%Y 2>/dev/null)
dia_ref=$(date -d "$1" +%-j 2>/dev/null)
# También necesitamos mes y día para comparaciones más precisas
mes_ref=$(date -d "$1" +%-m 2>/dev/null)
dia_mes_ref=$(date -d "$1" +%-d 2>/dev/null)

# +%Y%m%d: Crea un número largo con el año, mes y día (ej. 2024-02-24 es 20240224) para comparar fechas fácilmente.
fecha_ref_num=$(date -d "$1" +%Y%m%d 2>/dev/null)

# Comprobamos que la fecha sea válida (si date no pudo convertirla, las variables estarán vacías), -z devolverá true si la longitud es 0
if [ -z "$ano_ref" ]; then 
    echo "La fecha no es válida, usa el formato correcto: YYYY-MM-DD"
    exit 1
fi

# Obtener datos de la fecha actual, esta vez sin -d porque queremos la actual y date ya devuelve esa misma.
ano_act=$(date +%Y)
dia_act=$(date +%-j)
mes_act=$(date +%-m)
dia_mes_act=$(date +%-d)
hora_act=$(date +%-H) # Hora sin ceros (del 0 al 23)
min_act=$(date +%-M) # Minutos sin ceros (del 0 al 59)
fecha_act_num=$(date +%Y%m%d) # Convertimos en número largo para comparar con la fecha de referencia

# Imprimir por pantalla y comprobar las fechas para evitar fechas futuras
echo "Fecha introducida por el usuario: $1"
echo "Fecha actual del sistema: $(date +%Y-%m-%d)"

# Comparamos las fechas usando los numeros creados, con greater than (-gt) vemos si la fecha de referencia (la introducida por el usuario) es mayor que la fecha del dia actual
# Si es mayor imprime con echo un mensaje de error y sale del script con exit 1
if [ "$fecha_ref_num" -gt "$fecha_act_num" ]; then
    echo "La fecha introducida es futura."
    exit 1
fi

# Cálculos matemáticos de las diferencias en años, días y minutos usando aritmética de enteros, en bash para operaciones aritméticas se usa $((...))
anos=$(( ano_act - ano_ref )) # Diferencia bruta de años (sin mes ni dia)

# Se ajustan los años si aún no ha llegado dicho día en el año actual
if [ $mes_act -lt $mes_ref ] || ([ $mes_act -eq $mes_ref ] && [ $dia_mes_act -lt $dia_mes_ref ]); then
    anos=$(( anos - 1 )) #como aún no "cumplió" el año se resta uno
fi

# Función para determinar si un año es bisiesto
es_bisiesto() {
    local ano=$1
    if (( ano % 400 == 0 )) || (( ano % 4 == 0 && ano % 100 != 0 )); then
        return 0 # Es bisiesto
    else
        return 1 # No es bisiesto
    fi
}

# Calcular días dentro del último año
if [ $mes_act -eq $mes_ref ] && [ $dia_mes_act -eq $dia_mes_ref ]; then
    # hoy se suma un año, por lo que días = 0
    # de esta forma para el mismo dia y mes en años distintos los dias son 0, aunque haya bisiestos por el medio
    dias=0
else
    # Calcular días desde la última vez que se sumó el año
    dias=$(( dia_act - dia_ref ))
    
    # Si el día actual es menor al de la fecha dada, se debe ajustar (no puede ser negativo)
    if [ $dias -lt 0 ]; then
        # Necesitamos saber si el año del último cumpleaños fue bisiesto
        ano_ultimo_cumple=$((ano_ref + anos))
        
        # Determinamos si un año es bisiesto
        es_bisiesto()
        
        # Determinar días del año del último cumpleaños
        if es_bisiesto $ano_ultimo_cumple; then
            dias_ano_ultimo_cumple=366
        else
            dias_ano_ultimo_cumple=365
        fi
        
        dias=$(( dias + dias_ano_ultimo_cumple ))
    fi
fi

# Minutos transcurridos en el día de hoy (son los minutos desde que empezó el día)
# Si es el mismo día que la fecha de referencia, hay que restar los minutos de referencia
if [ "$fecha_ref_num" -eq "$fecha_act_num" ]; then
    # Si es el mismo día se calcula la diferencia de minutos
    minutos_ref=$(( $(date -d "$1" +%-H) * 60 + $(date -d "$1" +%-M) ))
    minutos_act=$(( hora_act * 60 + min_act ))
    minutos=$(( minutos_act - minutos_ref ))
else
    # Si son días diferentes, se usan los minutos desde el inicio del día actual
    minutos=$(( hora_act * 60 + min_act ))
fi

# Validación de minutos, los minutos no deben ser negativos ni superiores a 1440
if [ $minutos -lt 0 ] || [ $minutos -gt 1440 ]; then
    # Ajustar si es necesario (por ejemplo, si minutos_act < minutos_ref)
    if [ $minutos -lt 0 ]; then
        minutos=$(( minutos + 1440 ))
        dias=$(( dias - 1 ))
        # Si días se vuelve negativo, ajustar años (aunque esto no debería pasar)
        if [ $dias -lt 0 ]; then
            dias=$(( dias + 365 ))
            anos=$(( anos - 1 ))
        fi
    else
        echo "Error, los minutos calculados están fuera de rango."
        exit 1
    fi
fi

# Imprimir resultados calculados con echo, mostrando años, días dentro del último año y minutos dentro del último día
echo "------------------------------"
echo "Han pasado:"
echo "$anos años"
echo "$dias días (dentro del último año)"
echo "$minutos minutos (dentro del último día)"
