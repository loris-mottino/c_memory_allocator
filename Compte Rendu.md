#Compte Rendu TP Allocateur de mémoire
#Mottino & Abecassis & Verrier

##Modélisation :

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

###Schéma récapitulatif : 
![diagramme](https://image.noelshack.com/fichiers/2021/53/7/1609680861-chart.png)

- En noir : header
- En bleu : zone libre
- En rouge : zone occupée

##Tests :

Afin de vérifier que notre programme fonctionne, nous avons rédigé des tests.
Ces tests nous ont permis de mettre en lumière des erreurs dans notre programme et de les corriger.
Vous pouvez retrouver ces tests dans le fichier test.c qui est commenté, afin d'éxpliquer le but de chaque test.


##Extensions :

Nous avons choisis de coder deux fonctions de recherche d’espace libre, à allouer, supplémentaires. L’une cherchant une espace libre avec le meilleur ajustement (mem_fit_best) et l’autre avec le moins de résidu (mem_fit_worst).
Pour l’implémentation de ces deux fonctions, nous avons utilisé le même procédé.
En premier lieu, nous faisons appel à mem_fit_first, afin de nous assurer qu’il existe bien au moins un espace assez grand à allouer pour l'utilisateur. Ensuite nous parcourons tout l’espace mémoire à partir de cet espace afin de vérifier si il existe un espace tel que :

- le résidu non alloué de cet espace soit le plus petit possible ( mem_fit_best )
- le résidu non alloué de cet espace soit le plus grand possible ( mem_fit_worst )

Ces deux nouvelles méthodes de recherches impliquent de parcourir intégralement l’espace mémoire à chaque recherche, ce qui augmente grandement la complexité de l’algorithme et donc son temps d'exécution. Leur intérêt n’est donc pas dans la rapidité d'exécution mais dans l’optimisation de la mémoire.

###Cas où mem_fit_best optimise la mémoire :

Supposons qu’il ne reste que deux espaces (appelons les A et B) en mémoire , tel que:

  - taille A > taille B.
  - A avant B en mémoire

Lorsque l’utilisateur nous demande de lui allouer successivement deux blocs mémoire 
(appelons les 1 et 2) tel que :

  - taille 1 < taille 2
  - taille 1 < taille B
  - taille 2 > taille B
  - taille 1 + taille 2 > taille A
  - 1 demandé avant 2.

  Si nous utilisons la méthode mem_fit_first, alors nous allons alloué A pour 1 et lorsque l’utilisateur va nous demander 2 alors on ne pourra pas lui allouer car plus aucun bloc de taille suffisante ne sera disponible.
	Si nous utilisons la méthode mem_fit_worst, alors nous allons alloué A pour 1 et nous nous retrouvons avec le même problème.
  
Cependant, si nous utilisons la méthode mem_fit_best alors nous allons B pour 1, et nous pourrons ensuite allouer A pour 2.


###Cas où mem_fit_worst optimise la mémoire :

Supposons qu’il ne reste que deux espaces (appelons les A et B) en mémoire , tel que:

  - taille A < taille B.
  - A avant B en mémoire

Lorsque l’utilisateur nous demande de lui allouer successivement trois blocs mémoire 
(appelons les 1, 2 et 3) tel que :

  - taille 1 < taille 3 < taille 2
  - taille 2 < taille A < taille B
  - taille 1 + taille 2 > taille B
  - taille 1 + taille 3 < taille B
  - taille 2 + taille 3 > taille A
  - taille 1 + taille 2 > taille A
  - taille 1 + taille 3 > taille A
  - taille 2 + taille 3 > taille B
  - 1 demandé avant 2 et 2 demandé avant 3.

  Si nous utilisons la méthode mem_fit_first, alors nous allons alloué A pour 1 et B pour 2, mais lorsque l’utilisateur voudra un espace pour 3, il alors on ne pourra pas lui allouer car plus aucun bloc de taille suffisante ne sera disponible.
  Si nous utilisons la méthode mem_fit_bestt, alors nous allons alloué A pour 1, puis B pour 2 et nous nous retrouvons avec le même problème.
  
Cependant, si nous utilisons la méthode mem_fit_worst alors nous allons B pour 1, puis A pour 2 et nous pourrons ensuite allouer le résidu de B pour 3.


Par ailleurs, il existe aussi des situations où mem_fit_worst et mem_fit_best n'apportent pas de réel intérêt au niveau de l'optimisation de la mémoire.
exemple simple : si on a trois espaces A, B et C de même taille.
Dans ces cas, la méthode mem_fit_first reste la meilleure option puisqu’elle possède un temps d'exécution théorique bien moins important.


