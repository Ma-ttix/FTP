#!/bin/bash

# Vérification du nombre d'arguments
if [ "$#" -ne 2 ]; then
    echo "Usage : $0 fichier1.txt fichier2.txt"
    exit 1
fi

FILE1="$1"
FILE2="$2"

# Vérification que les fichiers existent
if [ ! -f "$FILE1" ] || [ ! -f "$FILE2" ]; then
    echo "Erreur : l'un des fichiers n'existe pas."
    exit 1
fi

# Comparaison des fichiers
if diff -q "$FILE1" "$FILE2" > /dev/null; then
    echo "Les fichiers sont identiques : c'est bon."
else
    echo "Les fichiers sont différents. Détails des différences :"
    diff "$FILE1" "$FILE2"
fi
