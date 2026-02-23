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
	&& apt remove -y nodejs npm libnode-dev \
	&& curl -fsSL https://deb.nodesource.com/setup_22.x | bash - \
	&& apt install -y nodejs \
	&& ARCH=$(uname -m | sed 's/x86_64/amd64/;s/aarch64/arm64/') \
	&& curl -fsSL "https://github.com/mikefarah/yq/releases/latest/download/yq_linux_${ARCH}" -o /usr/local/bin/yq \
	&& chmod +x /usr/local/bin/yq \
	&& rm -rf /var/lib/apt/lists/*

# Set default shell to zsh
SHELL ["/bin/zsh", "-c"]