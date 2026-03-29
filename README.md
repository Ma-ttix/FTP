# FTP SERVER
## Git
Le lien GitHub de ce projet: [Git](https://github.com/Ma-ttix/FTP)
## Compilation
Un simple appel à `make` réalise la compilation totale du projet, mais 3 options de compilation sont aussi disponibles:
* **TALK**: compiler avec cette option (et n'importe quelle valeur) ajoute des affichages supplémentaires lors de l'exécution
* **NPROC**: cette option sert à faire varier le nombre de processus exécutants appartenant au pool de processus de chaque serveur esclave
* **NBSLAVES**: cette option sert à faire varier le nombre de serveurs esclaves\
\
La compilation par défaut `make` est équivalent à `make NPROC=10 NBSLAVES=3`

## Exécution
Une fois le projet compilé, il suffit de lancer le serveur grâce à l'exécutable `ftpservermaitre`, puis de lancer le client (privilégier l'exécution dans un autre terminal par soucis de clarté (notamment si l'option TALK est activé)) avec `ftpclient` qui prend en argument l'host (ex: localhost)\
Pour réaliser le transfert d'un fichier, il suffit de taper la commande get dans le client suivi du nom de fichier qu'on souhaite récupérer (ce fichier doit se trouver dans le répertoire *server/*), ainsi ce dernier sera transféré dans le répertoire *client/*