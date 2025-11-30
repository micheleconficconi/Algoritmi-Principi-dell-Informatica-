Generatore di Test Pubblico Prova Finale API 2024
*************************************************

AVVERTENZE:
 - Gli eseguibili in oggetto sono forniti solo come convenienza.
 - Sono state prese misure precauzionali di natura tecnica per ostacolare
   l'ispezione del codice negli eseguibili.

Per eseguire il generatore di test, seguire le istruzioni specifiche per il
vostro sistema operativo.

# Windows

Sono supportati Windows 10 e le versioni successive per processori Intel a
64 bit.

1. Aprire il prompt dei comandi (cmd.exe, non Powershell)
2. Digitare "cd" (senza virgolette) seguito da uno spazio e dal percorso dove
   si trova questo file. Per esempio, se questo file si trova nella cartella
   Download, e la vostra cartella utente è C:\Users\Studente,
   dovete digitare "cd C:\Users\Studente\Download".
3. Digitare "test_gen_2024_w64.exe" per eseguire il generatore. I comandi
   generati verranno stampati sul terminale. Per scriverli a file invece
   digitate "test_gen_2024_w64.exe > nomefile.txt", sostituendo "nomefile.txt"
   col nome file desiderato.
   
# Linux

È supportata Ubuntu 22.04 per processori Intel a 64 bit.

1. Aprire il Terminale della vostra distribuzione.
2. Digitare "cd" (senza virgolette) seguito da uno spazio e dal percorso dove
   si trova questo file. Per esempio, se questo file si trova nella cartella
   Downloads, e la vostra cartella utente è /home/studente,
   dovete digitare "cd /home/studente/Downloads".
3. Digitare "test_gen_2024_linux" per eseguire il generatore. I comandi
   generati verranno stampati sul terminale. Per scriverli a file invece
   digitate "test_gen_2024_linux > nomefile.txt", sostituendo "nomefile.txt"
   col nome file desiderato.
   
# macOS

È supportato macOS Big Sur 11.0 e le versioni successive per processori Intel
a 64 bit e per processori Apple Silicon.

1. Aprire Terminale.app
2. Digitare "cd" (senza virgolette) seguito da uno spazio e dal percorso dove
   si trova questo file. Per esempio, se questo file si trova nella cartella
   Downloads, e la vostra cartella utente è /Users/studente,
   dovete digitare "cd /Users/studente/Downloads".
4. Digitare "xattr -c test_gen_2024_macos". Questo comando elimina l'attributo
   quarantena aggiunto automaticamente ai file scaricati da Internet.
   Questo attributo impedirebbe l'esecuzione di eseguibili non firmati da Apple
   come quello da noi fornito.
   Siccome siamo tirchi non intendiamo pagare 100$ a Apple per firmare un
   singolo eseguibile.
3. Digitare "test_gen_2024_macos" per eseguire il generatore. I comandi
   generati verranno stampati sul terminale. Per scriverli a file invece
   digitate "test_gen_2024_macos > nomefile.txt", sostituendo "nomefile.txt"
   col nome file desiderato.

# Opzioni disponibili

È possibile specificare opzioni aggiuntive al generatore aggiungendole
alla riga di comando. Eseguire il generatore con l'opzione "--help" per
scoprire le opzioni disponibili. Quindi, per esempio, sotto Linux al terminale
dovete digitare:

    ./test_gen_2024_linux --help

e poi premere invio. Per gli altri sistemi operativi basta sostituire il nome
dell'eseguibile in modo corretto.
