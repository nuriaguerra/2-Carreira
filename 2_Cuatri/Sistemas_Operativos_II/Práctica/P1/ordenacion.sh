#!/bin/bash

#CÓDIGO DE MARCOS GARCÍA BLANCO Y NURIA GUERRA CASAL
#Uso: ./ordenacion.sh directorio [-a|-b|-c|-d|-e]

#Comprobamos que se hayan pasado dos argumentos, el directorio y la opción
if [ $# -ne 2 ]; then
	echo "Error, número incorrecto de parámetros"
	echo "Uso: $0 directorio [-a|-b|-c|-d|-e]"
	exit 1
fi

#Guardamos el directorio en una variable y la opcion en otra
directorio=$1
opcion=$2

#Verificamos que el directorio que se pasa existe usando -d (devuelve verdadero si el archivo existe y es un directorio)
#Y si que es un directorio con permisos de lectura
if [[ ! -d "$directorio" || ! -r "$directorio" ]]; then
	echo "Error, $directorio no es un directorio existente o no tiene permiso de lectura "
	exit 1
fi

#Cambiamos al directorio que nos interesa, de esta forma se puede usar * en lugar de la ruta completa
#En caso de fallar cd termina el script con exit 1
cd "$directorio" || exit 1

#Función -a, para ordenar por número de caracteres del nombre
ordenar_caracteres(){
	echo "Ordenando por número de caracteres los nombres de los ficheros (menor a mayor)"

	#Creamos un archivo temporal para guardar los resultados com mktemp
	temp=$(mktemp)

	#Recorremos todos los archivos del directorio actual
	#El * se expande a todos los archivos (excepto ocultos)
	for archivo in *; do
		#Comprobamos que el archivo existe (por si el directorio está vacio)
		# -e comprueba si el archivo existe
		if [ -e "$archivo" ]; then
			#calculamos la longitud del nombre usando ${#variable} que devuelve el número de caracteres
			longitud=${#archivo}

			#Guardamos en el temporal: "longitud nombre" para facilitar la ordenacion posterior
			echo "$longitud $archivo" >> "$temp"
		fi
	done

	#Ordenamos numéricamente por el campo de la longitud
	# sort -n ordena numericamente de nemor a mayor secuencialmente
	# | toma la salida del comando de la izquierda y la pasa como entrada al comando de la derecha
	# read lee una línea de entrada y la divide en campos, -r no interpreta barras invertidas \ para evitar saltos de linea, longitud (primera parte) y nombre (resto despues del primer espacio) son variables
	sort -n "$temp" | while read -r longitud nombre; do

		#Mostramos por pantalla con formato
		# %3d reserva 3 espacios para el numero de la longitud
		printf " [%3d caracteres] %s\n" "$longitud" "$nombre"
	done

	#eliminamos el archivo temporal
	rm -f "$temp"
	echo "" #Espacio en blanco para que no se imprima todo seguido
}

#Funcion -b, ordenar por nombre alfabeticamente
ordenar_nombreInv(){
	echo "Ordenando por nombre de los archivos (alfabéticamente invertido)"
	#ls "$directorio" | rev | sort | rev

	temp=$(mktemp)
	for archivo in *; do
		if [ -e "$archivo" ]; then

			#Invertimos el nombre de cada archivo usando el comando rev
			# rev invierte el orden de los caracteres de cada linea
			nombreInv=$(echo "$archivo" |rev)

			#Guardamos el nombre_invertido y nombre_original en el archivo
			echo "$nombreInv $archivo" >> "$temp"
		fi
	done

	#Ordenamos alfabeticamente por el nombre invertido y mostramos
	sort "$temp" | while read -r invertido original; do
		echo " $original (invertido: $invertido)"
	done

	#Eliminamos el archivo temporal
	rm -f "$temp"
	echo ""
}

#Funcion -c, ordenar por los últimos 4 dígitos del inode
ordenar_inode(){
	echo "Ordenando por los últimos 4 dígitos del inode (de mayor a menor)"

	temp=$(mktemp)

	# ls -i muestra el número de inode antes del nombre
	ls -i | while read -r inodo nombre; do

		#Extraemos los últimos 4 dígitos del inodo
		# ${inodo: -4} toma los últimos 4 caracteres
		ultimos="${inodo: -4}"

		#Guardamos "ultimos_digitos inodo nombre" en el archivo
		echo "$ultimos $inodo $nombre" >> "$temp"
	done

	#Ordenamos de mayor a menor por los últimos 4 dígitos
	#sort -r ordena de mayor a menor
	#sort -n ordena numéricamente
	sort -rn "$temp" | while read -r ultimos inodo nombre; do
		printf " inode: %s (últimos 4: %s) -> %s\n" "$inodo" "$ultimos" "$nombre"
	done

	#Eliminamos el archivo temporal
	rm -f "$temp"
	echo ""
}

#Funcion -d, ordenar por tamaño y permisos del propietario
ordenar_permisos(){
	echo "Ordenando por tamaño y agrupado por permisos (-rwx del propietario)"

	temp=$(mktemp)

	#ls -l muestra permisos, numero enlaces, propietario, grupo, tamaño...
	# Guardamos la salida de ls -l en un archivo temporal
    	ls -l > "$temp.ls"

   	 # Leemos línea por línea
    	while read linea; do

	        #Evitamos la linea que empieza por "total", la primera linea de ls -l
        	if [[ "$linea" != total* ]]; then
        	
        		# Limpiamos espacios múltiples
        		# tr -s ' ' comprime múltiples espacios consecutivos en un solo espacio, tr es el comando de traducción, -s es para comprimir y seguidamente lo que se quiere comprimir (en este caso espacios)
			linea_limpia=$(echo "$linea" | tr -s ' ')
			
			# Extraemos los permisos (primer campo)
			# cut -d' ' -f1, para cada línea de entrada, la divide en campos usando espacios como separadores, y muestra solo el primer campo.
			permisos=$(echo "$linea_limpia" | cut -d' ' -f1)
			
			# Extraemos el tamaño (quinto campo)
			tam=$(echo "$linea_limpia" | cut -d' ' -f5)
			#Extraemos el nombre (a partir del campo 9)
			nombre=$(echo "$linea_limpia" | cut -d' ' -f9-)

			#Se necesitan los caracteres 2,3,4 (pertenecientes al propietario)
		    	#${variable:posicion:longitud}, se empieza en 1 y se toman 3 caracteres
			permProp=${permisos:1:3}

            		# Guardamos en el temporal: "permisos tamaño nombre"
            		echo "$permProp $tam $nombre" >> "$temp"
        	fi
    	done < "$temp.ls"

	echo ""
	echo "Agrupación por permisos"

	#Obtenemos los permisos distintos
	#cut -d" " -f1 extrae el primer campo (permisos)
	#sort ordena alfabeticamente y uniq elimina duplicados
	for permiso in $(cut -d" " -f1 "$temp" | sort | uniq); do
		echo ""
		echo "Permisos del propietario: $permiso"

		#Mostramos solo los que tengan ese permiso y se ordenan por tamaño
		#grep "^permiso " Busca líneas que empiecen con el valor de $permiso seguido de un espacio (^ indica que es al principio de cada linea)
		#sort -k2 -n ordena numericamente por el segundo campo (tamaño)
		grep "^$permiso " "$temp" | sort -k2 -n | while read -r perm tam nom; do
			printf " Tamaño: %6d bytes -> %s\n" "$tam" "$nom"
		done
	done

	#Eliminamos el archivo temporal
	rm -f "$temp"
	echo ""
}

#Funcion -e, ordenar por inode y agrupar por mes de ultimo acceso
ordenar_mes(){
	echo "Ordenando por inode y agrupado por mes de último acceso"

	temp=$(mktemp)

	# ls -li --time=atime muestra
		# -l  formato largo
		# -i  muestra el numero de inode
		# --time=atime usa fecha de último acceso

	ls -li --time=atime > "$temp.ls"

	#Leemos linea a linea
	while read linea; do

		# Evitamos línea "total"
		if [[ "$linea" != total* ]]; then
			# Limpiamos espacios múltiples
			linea_limpia=$(echo "$linea" | tr -s ' ')

			# Extraemos inodo (primer campo)
			inodo=$(echo "$linea_limpia" | cut -d' ' -f1)

			# Extraemos el mes (campo 7)
			mes=$(echo "$linea_limpia" | cut -d' ' -f7)
			
			#Extraemos el nombre (campo 10)
			nombre=$(echo "$linea_limpia" | cut -d' ' -f10-)
                        echo "$mes $inodo $nombre" >> "$temp"
		fi
	done < "$temp.ls"

	echo ""
	echo "Agrupación por mes de último acceso:"

	# Recorremos todos los meses posibles en orden cronologico (en español como aparecen al hacer ls -li --time=atime)
	for mes in ene feb mar abr may jun jul ago sep oct nov dic; do

		#Verificamos que haya archivos en este mes, -c cuenta lineas que coinciden
		numArch=$(grep -c "^$mes " "$temp")
		if [ $numArch -gt 0 ]; then
			echo ""
			echo "Mes: $mes ($numArch archivos)"

			# Mostramos solo los de ese mes
			# grep "^$mes " "$temp" busca en el archivo temporal $ todas las líneas que empiezan con el mes actual del bucle
			#sort -k2 -n ordena las lineas por el segundo campo (inodo) numericamente
			grep "^$mes " "$temp" | sort -k2 -n | while read -r mes inodo nombre; do
				printf " inode: %s -> %s\n" "$inodo" "$nombre"
			done
		fi
	done

	rm -f "$temp"
	echo ""
}

#Usamos un case para manejar las distintas opciones
case $opcion in
	-a) ordenar_caracteres ;;
	-b) ordenar_nombreInv ;;
	-c) ordenar_inode ;;
	-d) ordenar_permisos ;;
	-e) ordenar_mes ;;
	*) #cualquier otra opción que no sea las anteriores
		echo "Opción inválida"
		echo "Uso: $0 directorio [-a|-b|-c|-d|-e]"
		exit 1
		;;
esac
