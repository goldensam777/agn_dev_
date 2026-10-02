# .forge/Dockerfile — Environnement de Sandbox & Isolation pour Agents
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# 1. Dépendances de base et outils de build
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    clang \
    clang-tidy \
    lld \
    lldb \
    gdb \
    valgrind \
    cmake \
    ninja-build \
    git \
    curl \
    ca-certificates \
    python3 \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# 2. Installation de Node.js 22 LTS
RUN curl -fsSL https://deb.nodesource.com/setup_22.x | bash - \
    && apt-get install -y --no-install-recommends nodejs \
    && rm -rf /var/lib/apt/lists/*

# 3. Installation de Rust
ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo \
    PATH=/usr/local/cargo/bin:$PATH

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain stable \
    && chmod -R a+w /usr/local/rustup /usr/local/cargo \
    && cargo --version \
    && rustup component add clippy

# 4. Dossier de travail sécurisé
WORKDIR /workspace

# Utilisateur non-root optionnel ou exécution sous sandbox
CMD ["bash"]
