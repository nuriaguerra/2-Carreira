#!/bin/bash
#CÓDIGO DE MARCOS GARCÍA BLANCO Y NURIA GUERRA CASAL

# Verificación de argumentos
if [ $# -ne 1 ]; then
    echo "Uso: ./clientes.sh <archivo_agenda>"
    exit 1
fi

# Almacenamos el argumento en una variable para que el script sea más legible
ARCHIVO=$1

# Función auxiliar para comprobar si el archivo existe y si tenemos 1os permisos son adecuados para su uso (lectura y escritura)
verificar_agenda() {
    # Se comprueba con las opciones: -f: existe, -r: lectura, -w: escritura si tenemos permisos y si existe el archivo

    if [[ -f "$ARCHIVO" && -r "$ARCHIVO" && -w "$ARCHIVO" ]]; then
        return 0
    else
        echo "Error, el archivo no existe o no tienes permisos suficientes (lectura/escritura)."
        return 1
    fi
}

# Bucle del menú principal con las opciones para el uso
while true; do
    echo ""
    echo "Agenda de Clientes: $ARCHIVO"
    echo "1. Crear nueva agenda"
    echo "2. Registrar nuevo cliente"
    echo "3. Buscar por nombre"
    echo "4. Buscar por email"
    echo "5. Modificar entrada"
    echo "6. Borrar entrada"
    echo "7. Salir"

    # Leemos la opción del usuario con read y almacenamos en la variable opcion
    # -p muestra el mensaje sin salto de línea para que el usuario introduzca su opción
    read -p "Selecciona una opción: " opcion

    # Usamos un case para manejar las diferentes opciones del menú
    case $opcion in
        1)
            # Se crea el archivo de la agenda con touch y el nombre del mismo
            touch "$ARCHIVO"
            echo "Agenda '$ARCHIVO' creada correctamente."
            ;;
        2)
            # No se necesita verificar, porque '>>' crea el archivo si no existe
            # Se pide al usuario el nombre y el email del cliente, y se guarda en el archivo con el formato "nombre#email"
            read -p "Nombre: " nombre
            read -p "Email: " email
            echo "$nombre#$email" >> "$ARCHIVO"
            echo "Cliente añadido."
            ;;
        3)
            # Se comprueba si existe la agenda, si no existe se muestra un mensaje de error y se vuelve al menú
            if verificar_agenda; then
                # Se lee usando read el nombre a buscar
                read -p "Nombre a buscar: " nombre
                # Como es nombre lo que se busca se usa grep para encontrar dicha cadena y un asterisco al final que 
                # asegura que es el porimer campo de la línea
                # Con el -i se ignoran mayusculas y muinsculas para que la búsqueda sea más flexible
    
                grep -i "^$nombre#" "$ARCHIVO" || echo "No encontrado."
            fi
            ;;
        4)
            # Se hace de la misma forma que el 3 pero buscando un # antes del email para asegurar que es el segundo campo de la línea
            if verificar_agenda; then
                read -p "Email a buscar: " email
                grep -i "#$email$" "$ARCHIVO" || echo "No encontrado."
            fi
            ;;
        5)
            # Se verifica la agenda y se pide el nombre del cliente a modificar, si no se encuentra se muestra un mensaje de error
            # Si se encuentra, se pide el nuevo email y se usa sed para modificar la línea
            if verificar_agenda; then
                read -p "Nombre del cliente a modificar: " nombre
                # grep -q busca la cadena sin mostrarla, si se encuentra devuelve 0 y si no se encuentra devuelve 1
                if grep -q -i "^$nombre#" "$ARCHIVO"; then
                    # Si la encuentra, se pide el nuevo email y se usa sed para modificar la línea
                    read -p "Nuevo email: " nuevo_email

                    # sed -i "s/^$nombre#.*/$nombre#$nuevo_email/" "$ARCHIVO" busca la línea que empieza con el nombre seguido de un # 
                    # y reemplaza toda la línea por el nuevo formato con el nuevo email
                    # -i hace que la modificación se haga directamente en el archivo, sin necesidad de crear un archivo temporal
                    # La s indica que es una sustitución, el patrón a buscar es "^$nombre#.*" (línea que empieza con el nombre seguido de un # y cualquier cosa después)
                    # La / es el delimitador. sed necesita un carácter para separar el comando (s), el patrón a buscar, y el patrón de reemplazo.
                    # El patrón de reemplazo es "$nombre#$nuevo_email", que es el nuevo formato con el nuevo email
                    # Se indica el archivo al final para que sed sepa dónde hacer la sustitución
                    sed -i "s/^$nombre#.*/$nombre#$nuevo_email/" "$ARCHIVO"
                    echo "Cliente modificado."
                else
                    echo "Cliente no encontrado."
                fi
            fi
            ;;
        6)
            # Se comprueba la agenda y se pide el nombre del cliente a borrar, si no se encuentra se muestra un mensaje de error
            if verificar_agenda; then
                read -p "Nombre del cliente a borrar: " nombre
                # Se busca con grep -q si el cliente existe, si existe se usa sed para eliminar la línea completa que contiene el nombre del cliente
                if grep -q -i "^$nombre#" "$ARCHIVO"; then
                    # sed -i "/^$nombre#/d" "$ARCHIVO" busca la línea que empieza con el nombre seguido de un # y la elimina (d) del archivo
                    sed -i "/^$nombre#/d" "$ARCHIVO"
                    echo "Cliente borrado."
                else
                    echo "Cliente no encontrado."
                fi
            fi
            ;;
        7)
            # Sale del script con exit 0, indicando que la ejecución ha sido exitosa
            exit 0
            ;;
        *)
            echo "Opción no válida."
            ;;
    esac
done
