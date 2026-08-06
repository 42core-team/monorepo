FROM node:24-alpine AS builder

ARG INCLUDE_DEV_ALIAS=false
WORKDIR /build

COPY .version-builds /build/sources
COPY scripts/build-versioned-site.sh scripts/rewrite-base-path.mjs /build/scripts/

RUN --mount=type=cache,target=/root/.npm \
	/build/scripts/build-versioned-site.sh \
		/build/sources \
		/build/site \
		"${INCLUDE_DEV_ALIAS}"

FROM nginx:alpine

COPY --from=builder /build/site /usr/share/nginx/html
COPY nginx.conf /etc/nginx/nginx.conf
RUN nginx -t

EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
