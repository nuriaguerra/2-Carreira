#!/bin/bash

#CÓDIGO DE MARCOS GARCÍA BLANCO Y NURIA GUERRA CASAL
#Comprobar número de parámetros recibidos
# $# contiene el número de parámetros que se pe pasan al script, -ne es "not equal"
if [ $# -ne 2 ]; then
	echo "Error, número incorrecto de parámetros introducidos"
	echo "Uso: $0 dir_origen dir_destino"
	exit 1
fi

#Guardar parámetros en variables
origen="$1"
destino="$2"

#Comprobar el directorio origen
# -d comprueba que se trate de un directorio y -r comprueba permisos de lectura

if [ ! -d "$origen" ] || [ ! -r "$origen" ]; then
	echo "Error, el primer parámtro recivido debe ser un directorio con permisos de lectura"
	echo "Uso: $0 dir_origen dir_destino"
	exit 1
fi

#Comprobar el directorio detino
# -d comprueba que se trate de un directorio y -w comprueba que se tenga permiso de escritura

if [ ! -d "$destino" ] || [ ! -w "$destino" ]; then
	echo "Error, el segundo parámetro debe ser un directorio con permisos de escritura."
	echo "Uso: $0 dir_origen dir_destino"
	exit 1
fi

#Comprobar que no existen directorios llamados escena20 a escena50
# seq 20 50, genera la secuencia de números del 20 a 50
# -d "$destino/escena$i" comprueba si existe un directorio llamado escenaXX
for i in $(seq 20 50); do
	if [ -d "$destino/escena$i" ]; then
		echo "Error, ya existe el directorio $destino/escena$i"
		echo "El destino no debe contener directorios escena20 a escena50"
		exit 1 #se detiene si existe para no sobrescribir nada
	fi
done

#Obtener una lista de fechas únicas
#Creamos un array vacío para guardar fechas
fechas=()

#Recorremos los ficheros del directorio origen
for fichero in "$origen"/*; do
	#Extraemos con basename solo el nombre sin ruta
	nombre=$(basename "$fichero")

	#Extraemos la parte posterior a "escena_XX_"
	#Bash Parameter Expansion: "${variable#patrón}" elimina desde el inicio hasta que coincide con el patrón
	resto=${nombre#escena_*_}

	#Extraemos fecha (parte anterior de @)
	# "${variable%%patrón}" elimina desde el final lo que coincide con el patrón más largo
	fecha=${resto%%@*}

	#Añadimos fecha al array si no está incluida
	#Con " ${fechas[@]} " se convierte el array en un string separado por espacios
	#Con =~ se verifica que existan coincidencias
	if [[ ! " ${fechas[@]} " =~ " ${fecha} " ]]; then
		fechas+=("$fecha") #añade la fecha al array
	fi
done

#Crear la estructura de carpetas
for i in $(seq 20 50); do
    for fecha in "${fechas[@]}"; do

        # Creamos directorio escenaXX/fecha
	#mkdir -p crea carpetas y todos los subdirectorios necesarios
        mkdir -p "$destino/escena$i/$fecha"
    done
done

#Copiar y renombrar archivos
for fichero in "$origen"/*; do

    nombre=$(basename "$fichero")

    #Extraer partes del nombre, quita la palabra escena_ del nombre
    temp=${nombre#escena_}

    #Extraer número de escena, toma el número de la escena
    escena=${temp%%_*}

    # Solo escenas 20–50, saltamos cualquier archivo que no esté entre estos valores
    if [ "$escena" -lt 20 ] || [ "$escena" -gt 50 ]; then
        continue
    fi

    # Extraer fecha y hora
    resto=${temp#*_} #elimina el número de escena y _
    fecha=${resto%%@*} #la fecha antes de @
    hora_cam=${resto#*@} #todo despues de la @, hora.cámara

    # Extraer hora (antes del punto)
    hora=${hora_cam%%.*} #extrae la hora, antes del punto

    # Extraer cámara, después del punto ("CAM A", "CAM B", "Heat", "Noct")
    cam=${hora_cam##*.}

    #Definir nueva ruta y nuevo nombre
    nueva_ruta="$destino/escena$escena/$fecha/$cam"
    mkdir -p "$nueva_ruta" #crea carpeta y subcarpeta de la cámara

    #Copiar con nuevo nombre, solo la hora
    nuevo_nombre="escena_$hora"

    #copiar archivo a su destino con nuevo nombre
    cp "$fichero" "$nueva_ruta/$nuevo_nombre"

done

echo "Proceso completado correctamente."
