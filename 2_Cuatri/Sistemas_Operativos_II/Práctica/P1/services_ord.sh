#!/bin/bash

#CÓDIGO DE MARCOS GARCÍA BLANCO Y NURIA GUERRA CASAL
#Ordenar con sort alfabeticamente y con -u eliminar duplicados.
#Despues se guarda en el directorio /tmp/services_original usando el caracter ">"
sort -u ~/services_copia > /tmp/services_original

#Calcular el numero de lineas que había antes de eliminar duplicados y almacenarlas en una 
#variable 

#Se hace el 'word count' con -l para que cuente el numero de lineas,luego con el '<' se
#toma solo la entrada del archivo 
lineasAntesSort=$(wc -l < "$HOME/services_copia")

#Se hace lo mismo pero con el archivo nuevo
lineasDespuesSort=$(wc -l < /tmp/services_original)

#Calcular la diferencia usando $((...)) que se usa para expresiones aritmeticas
lineasEliminadas=$((lineasAntesSort-lineasDespuesSort))
#Calcular diferencia e imprimir por pantalla los resultados usando echo
echo "Se han eliminado $lineas_eliminadas líneas" 

#Comprobación de que /tmp/services original es igual a /etc/services usando los comandos diff y sort. 
#diff compara los ficheros linea por linea y sort los ordena y si los ordenamos podemos llegar
#a ver si son diferentes

#con el -s te avisa si son diferentes pero sin el no dice nada por eso se pone el -s
#con <() hace que se ejecute ese comando en un proceso a parte y se obtiene el resultado con un proceso temporal
diff -s <(sort /etc/services) /tmp/services_original
