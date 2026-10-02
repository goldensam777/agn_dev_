# Domaine C : Programmation Système & Syscalls Linux

> Ce document établit les règles d'interaction directe avec le noyau du système d'exploitation en C.

---

## 1. Gestion des Descripteurs de Fichiers & `O_CLOEXEC`

Tout descripteur de fichier ouvert (via `open`, `socket`, `pipe`) doit systématiquement utiliser le drapeau `O_CLOEXEC` :
```c
int fd = open("/path/to/data", O_RDONLY | O_CLOEXEC);
```
*Pourquoi ? Si l'application lance un sous-processus via `fork`/`exec`, le descripteur de fichier n'est pas fuité par inadvertance vers le programme enfant.*

---

## 2. Allocation Virtuelle par `mmap`

Pour les gros volumes de données (mémoire > 16 Mo, tables de hachage géantes, partage inter-processus) :
- Éviter `malloc` et allouer directement des pages virtuelles au noyau via `mmap` :
```c
size_t length = 64 * 1024 * 1024; // 64 Mo
void* addr = mmap(nullptr, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
if (addr == MAP_FAILED) {
    // Gestion d'échec
}

// Utilisation...

munmap(addr, length);
```

---

## 3. Sécurité dans les Gestionnaires de Signaux (*Signal Handlers*)

- **Règle absolue :** Ne JAMAIS appeler `malloc`, `free`, `printf`, ou toute fonction non réentrante dans un gestionnaire de signal POSIX (`SIGINT`, `SIGTERM`).
- Seules les variables de type `volatile sig_atomic_t` ou les appels système explicitement async-signal-safe (`write`, `close`) sont tolérés.

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Tout appel `open` ou création de socket inclut-il `O_CLOEXEC` ?
- [ ] Les retours des appels système (`open`, `read`, `write`, `mmap`) sont-ils vérifiés contre `-1` ou `MAP_FAILED` ?
- [ ] Les gestionnaires de signaux sont-ils rigoureusement *async-signal-safe* ?
