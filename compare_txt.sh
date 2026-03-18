#!/bin/bash

# Vérification du nombre d'arguments
if [ "$#" -ne 1 ]; then
    echo "To compare files with same name from directory server and client"
    echo "Usage : $0 nomFic"
    exit 1
fi

FILE1=server/"$1"
FILE2=client/"$1"

# Vérification que les fichiers existent
if [ ! -f "$FILE1" ] || [ ! -f "$FILE2" ]; then
    echo "Erreur : l'un des fichiers n'existe pas."
    exit 1
fi

# Comparaison des fichiers
if diff -q "$FILE1" "$FILE2" > /dev/null; then
    echo "Les fichiers $FILE1 $FILE2 sont identiques : c'est bon."
else
    echo "Les fichiers sont différents. Détails des différences :"
    diff "$FILE1" "$FILE2"
fi
