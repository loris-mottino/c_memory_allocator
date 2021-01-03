Compte Rendu TP Allocateur de mémoire
Mottino & Abecassis & Verrier

Modélisation :

Les structures pour le header de notre zone mémoire ainsi que pour les blocs libres étaient déjà fournis. Concernant le header, nous avons décidé de l’implémenter avec 3 données,
la taille de la zone mémoire total disponible ( size_t memory_size ),
un pointeur de fonction, qui pointera vers la fonction de recherche souhaitée ( mem_fit_function_t *fit );
et enfin une pointeur sur structure, qui pointe vers la première zone libre ( struct fb *list ).
Pour le bloc libre, deux données nous ont paru être nécessaire, la taille du bloc libre, total, ( Size_t size ),
ainsi qu’un pointeur vers la prochaine structure de bloc libre ( fb *next ).
Ainsi, il nous restait donc à choisir la manière d’implémenter les blocs occupés.
Grâce à la zone mémoire et aux zones libres nous pouvons déduire quelles zones sont occupées à condition de connaître la taille de chacunes d’entre elles.
C’est pour cela que nous avons choisis de mettre un size_t taille à chaque début de zone occupées,
nous permettant de parcourir la totalité des blocs présents en mémoire.

Schéma récapitulatif : 
![diagramme](https://image.noelshack.com/fichiers/2021/53/7/1609680861-chart.png)

En bleu : zone libre
En rouge : zone occupée

Tests :





Extensions :


