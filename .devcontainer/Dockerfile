FROM ubuntu:noble

# Update and install base packages
RUN apt-get update && apt-get install -y \
	git \
	curl \
	build-essential \
	clang \
	clang-format \
	npm \
	pkg-config \
	libssl-dev \
	libcurl4-openssl-dev \
	zsh \
	cmake \
	yq \
	&& apt remove -y nodejs npm libnode-dev \
	&& curl -fsSL https://deb.nodesource.com/setup_22.x | bash - \
	&& apt install -y nodejs \
	&& rm -rf /var/lib/apt/lists/*

# Install Go 1.26.0
ARG GO_VERSION=1.26.0
RUN ARCH=$(dpkg --print-architecture) && \
	curl -fsSL "https://go.dev/dl/go${GO_VERSION}.linux-${ARCH}.tar.gz" | tar -C /usr/local -xz
ENV PATH="/usr/local/go/bin:/root/go/bin:${PATH}"
ENV GOPATH="/root/go"

# Set default shell to zsh
SHELL ["/bin/zsh", "-c"]