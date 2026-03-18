# Compare pour chaque fichier passé en paramètre s'il est égal dans le dossier server/ et le dossier client/

# Vérification qu'au moins un argument est fourni
if [ "$#" -lt 1 ]
then
    echo "To compare files with same name from directory server and client"
    echo "Usage : $0 nomFic1 [nomFic2 ...]"
    exit 1
fi

# Boucle sur tous les arguments
for filename in "$@"
do
    FILE1="server/$filename"
    FILE2="client/$filename"

    # Vérification que les fichiers existent
    if [ ! -f "$FILE1" ] || [ ! -f "$FILE2" ]
    then
        echo "Erreur : l'un des fichiers ($filename) n'existe pas."
        continue  # passe au fichier suivant
    fi

    # Comparaison des fichiers
    if diff -q "$FILE1" "$FILE2" > /dev/null
    then
        echo "Les fichiers $FILE1 et $FILE2 sont identiques : c'est bon."
    else
        echo "Les fichiers $FILE1 et $FILE2 sont différents. Détails des différences :"
        diff "$FILE1" "$FILE2"
    fi
done