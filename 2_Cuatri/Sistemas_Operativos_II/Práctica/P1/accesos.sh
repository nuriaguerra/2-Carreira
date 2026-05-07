#!/bin/bash
#CÓDIGO DE MARCOS GARCÍA BLANCO Y NURIA GUERRA CASAL

# Comprobar que se pasan exactamente dos parámetros
# Con "$#" se obtiene el número de parámetros pasados al script, y se compara con -ne (not equal) con 2
# Luego con echo se muestra como se debe llamar al script para que funcione

if [ "$#" -ne 2 ]; then
    echo "Error: Número de parámetros incorrecto."
    echo "Uso: ./accesos.sh [-c, -t, GET|POST, -s, -o, IP] ruta/al/archivo/access.log"
    exit 1
fi

# Asignar variables a las opciones y al archivo de log para facilitar su uso posterior en el script y que 
# el código sea mas legible que usando "$1" y "$2" cada vez que se necesite usar la opción o el archivo de log.
OPCION="$1" # La opción que se ha pasado al script (GET, POST, -c, etc.)
ARCHIVO="$2" # El archivo de log que se ha pasado al script

# Comprobar que el segundo parámetro es un archivo y si tenemos permisos para poder leerlo, si no es un archivo o no tenemos 
# permisos de lectura sobre él entonces muestra un mensaje de error y sale con código que indicaa fallo

# Con el "! -f "$ARCHIVO"" se comprueba si el archivo existe y es un archivo regular
# Con "! -r "$ARCHIVO"" se comprueba si el archivo tiene permisos de lectura
# El if se ejecuta si alguna de las dos condiciones es verdadera, es decir, si el archivo no existe o no tiene permisos de lectura
if [ ! -f "$ARCHIVO" ] || [ ! -r "$ARCHIVO" ]; then
    echo "Error: El archivo '$ARCHIVO' no existe o no tiene permisos de lectura."
    echo "Uso: ./accesos.sh [-c, -t, GET|POST, -s, -o, IP] ruta/al/archivo/access.log"
    exit 1
fi

# Usar un case ... in para elegir entre las diferetes opciones que se pueden pasar al script
case $OPCION in
    -c) 
        echo "Opcion -c seleccionada"
        # Muestra los diferentes códigos de respuesta sin repetición y el número de veces que se produce cada uno.

        # Usamos cut para extraer la columna 9 (donde está el código de respuesta)
        # -d' ' indica que el separador es el espacio
        # -f9 indica que queremos la novena columna
        # sort | uniq -c se usa para contar cuántas veces aparece cada código de respuesta sin repetición
        # while read indica que lo que viene por la tuberia se va a leer linea a linea
        # como el -c de uniq pone el numero de veces que aparece antes de la linea se lee en 2 variables cantidad y código
        # Se muestra el resultado con echo indicando el código y la cantidad de veces que aparece ese código en el log.
        cut -d' ' -f9 "$ARCHIVO" | sort | uniq -c | while read -r cantidad codigo; do
            echo "Código $codigo: $cantidad veces"
        done
        ;;

    -t)
        echo "Opcion -t seleccionada"
        # Muestra el número de días para los que no hay ningún acceso al servidor, desde la fecha del primer acceso hasta la fecha del último
               
        # Extraemos la fecha del primer acceso (primera línea)
        # head -n 1 "$ARCHIVO": Lee solo la primera línea del fichero de log.
        linea_inicio=$(head -n 1 "$ARCHIVO")

        # Usamos una tuberia para almacenar solo la fecha en la variable del primer dia
        # Con echo se muestra la linea de inicio completa, con la salida de la impresión 
        # Usando cut -d'[' -f2 se obtiene la parte de la línea que está después del primer corchete '[', que es donde comienza la fecha.
        # Luego con cut -d':' -f1 se obtiene la parte de la fecha que está antes de la hora, es decir, el día, mes y año.
        # Finalmente, con sed 's/\// /g' se reemplazan las barras '/' por espacios para facilitar el procesamiento posterior con el comando date.
        # La g del sed indica que se reemplacen todas las ocurrencias de '/' en la cadena.
        primer_dia=$(echo "$linea_inicio" | cut -d'[' -f2 | cut -d':' -f1 | sed 's/\// /g')
        
        # Extraemos la fecha del último acceso (última línea) de la misma forma que la antrior pero usando tail -n 1 para leer la última línea del fichero de log.
        linea_fin=$(tail -n 1 "$ARCHIVO")
        ultimo_dia=$(echo "$linea_fin" | cut -d'[' -f2 | cut -d':' -f1 | sed 's/\// /g')

        # Convertimos las fechas para poder calcular las diferencias en días entre ellas usando date -d 
        inicio=$(date -d "$primer_dia" +%s)
        fin=$(date -d "$ultimo_dia" +%s)

        # Calculamos el número de días entre las dos fechas dividiendo la diferencia en segundos entre 86400 (número de segundos en un día)
        # sumando 1 para incluir ambos días en el conteo.
        dias_rango=$(( (fin - inicio) / 86400 + 1 ))
        
        # Con cut -d'[' -f2 "$ARCHIVO": Extrae todas las fechas de todas las líneas del archivo.
        # Luego con cut -d':' -f1 se obtiene solo la parte de la fecha que corresponde al día, mes y año.
        # Con sort | uniq se eliminan las fechas duplicadas para obtener solo los días únicos
        # Finalmente, con wc -l se cuenta el número de días únicos que hay en el archivo de log
        dias_con_acceso=$(cut -d'[' -f2 "$ARCHIVO" | cut -d':' -f1 | sort | uniq | wc -l)

        # Se imprime un resumen usando echo
        echo "Análisis desde: $primer_dia"
        echo "Análisis hasta: $ultimo_dia"
        echo "Días transcurridos: $dias_rango"
        echo "Días con registros: $dias_con_acceso"
        echo "Días SIN accesos registrados: $(( dias_rango - dias_con_acceso ))"
   
        ;;

    GET|POST)
        echo "Opcion $OPCION seleccionada"

        # Usamos grep para buscar exactamente "GET o "POST seguido de espacio.
        # Con cut extraemos la columna 9 usando separador de espacio indicado con el -d' '
        # -f9 para indicar que queremos la novena columna (código de respuesta).
        # grep -c cuenta directamente cuántas líneas coinciden exactamente con el número "200".
        total=$(grep "\"$OPCION " "$ARCHIVO" | cut -d' ' -f9 | grep -c "^200$")
        
        # Obtenemos la fecha actual con el formato "Mes Día Hora:Minuto:Segundo" usando date +"%b %d %H:%M:%S" para imprimirla
        fecha_actual=$(date +"%b %d %H:%M:%S")
        echo "$fecha_actual. Registrados $total accesos tipo $OPCION con respuesta 200."
        ;;

    -s)
        echo "Opcion -s seleccionada"
        # Resume el total de Datos enviados en KiB por cada mes.
        
        # Iterar sobre cada mes con un bucle for.
        for mes in Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec; do
            # Filtrar todas las líneas que contienen ese mes específico usando grep que busca patrones de texto en específico.
            # Con "/$mes/": Aquí buscas el mes rodeado de barras para asegurar que es el campo del mes
            # despues se indica el archivo en el que se va a buscar
            lineas_mes=$(grep "/$mes/" "$ARCHIVO")

            # Contar los accesos totales para ese mes usando 'wc -l' a la salida de una tubería.
            accesos=$(echo "$lineas_mes" | wc -l)

            # Sumar los bytes de ese mes con un bucle while anidado.
            total_bytes=0
            
            # Procesamos cada línea del mes para extraer la columna 10 (bytes).
            while read -r linea; do
                # Cogemos con el cut la columna 10 de cada línea, que es donde se encuentran los bytes enviados.
                bytes=$(echo "$linea" | cut -d' ' -f10)
                
            # <<< "$lineas_mes" se usa para alimentar el bucle while con cada línea de la variable lineas_mes. 
            # Es una forma de redirigir la variable como entrada estándar para el bucle while, permitiendo procesar cada línea individualmente.
            done <<< "$lineas_mes"

            # Convertimos a KiB dividiendo por 1024.
            # En Bash, la división entera redondea hacia abajo automáticamente.
            kib=$((total_bytes / 1024))
            
            echo "$kib KiB sent in $mes by $accesos accesses."
        done
        ;;
    

    -o)
        echo "Opcion -o seleccionada"
        # Ordena las líneas del fichero access.log por IP, y dentro de la misma IP, por bytes enviados de forma decreciente. 
        #El resultado lo almacenará en el fichero access ord.log

        # sort -t' ' indica que el separador es el espacio.
        # -k indica las claves por las que se va a ordenar, en este caso se ordena primero por la primera columna (IP) y luego por la décima columna (bytes enviados).
        # -k1,1 ordena por la primera columna (IP).
        # -k10,10nr ordena por la columna 10 (bytes) de forma Numérica y Reversa (descendente).
        # 1,1 y 10,10 indican que se ordena solo por esas columnas, no por las siguientes.
        # La n indica que es numerico, y la r indica que es en orden inverso (de mayor a menor).
        # Se redirige la salida al fichero access_ord.log.       
        sort -t' ' -k1,1 -k10,10nr "$ARCHIVO" > access_ord.log
        echo "Fichero 'access_ord.log' generado correctamente."
        ;;
        
    *)
        echo "Opcion IP seleccionada"
       # Muestra el total de accesos para cada día y los bytes enviados de la IP indicada.

        # Asignamos la IP que se ha pasado como opción a la variable IP para facilitar su uso posterior en el script.
        IP="$OPCION"
        
        # Buscamos todas las líneas que empiezan por la IP indicada seguida de un espacio ya que IP es el primer campo del log.
        lineas_ip=$(grep "^$IP " "$ARCHIVO")
        
        # Si la cadena devuelta está vacía (-z), la IP no existe en el log.
        if [ -z "$lineas_ip" ]; then
            echo "No se encontraron accesos para la IP: $IP"
        else
            echo "Resultados para la IP: $IP"
            
            # Igual que hicimos antes, sacamos los días únicos en los que esta IP tuvo actividad.
            dias=$(echo "$lineas_ip" | cut -d'[' -f2 | cut -d':' -f1 | sort | uniq)
            
            # Analizamos día a día recorriendo con un bucle for estos dias.
            for dia in $dias; do
                # Buscamos con grep ese día en específico para obtener la informacion de ese día.
                lineas_dia=$(echo "$lineas_ip" | grep "\[$dia:")
                
                # Contamos los accesos de esta IP en ese dia.
                accesos=$(echo "$lineas_dia" | wc -l)
                
                suma_bytes=0
                
                # Sumar los bytes de ese día en concreto (columna 10) recorriendo con un for todas las apariciones.
                for bytes in $(echo "$lineas_dia" | cut -d' ' -f10); do
                    suma_bytes=$((suma_bytes + bytes))
                done
                
                # Mostrar resultado de ese día.
                echo "Día $dia: Accesos = $accesos | Bytes enviados = $suma_bytes"
            done
        fi
        ;;
esac        
