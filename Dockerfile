# syntax=docker/dockerfile:1

# Build stage
FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build

# AOT compilation requires clang and zlib headers
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang zlib1g-dev \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY src/ ./src/

RUN dotnet publish src/kiwi.csproj \
      -c Release \
      -r linux-x64 \
      --self-contained true \
      -p:PublishAot=true \
      -p:PublishTrimmed=true \
      -p:StripSymbols=true \
      -p:DebugType=none \
      -p:IncludeNativeLibrariesForSelfExtract=true \
      -p:UseSystemTextJsonSourceGeneration=true \
      -p:JsonSerializerIsReflectionEnabled=false \
      -o /out

# Runtime stage
FROM debian:bookworm-slim

# kiwi-settings.json references ../lib relative to the binary, so the layout is:
#   /opt/kiwi/bin/kiwi
#   /opt/kiwi/bin/kiwi-settings.json
#   /opt/kiwi/lib/   ← standard library
COPY --from=build /out/kiwi               /opt/kiwi/bin/kiwi
COPY --from=build /out/kiwi-settings.json /opt/kiwi/bin/kiwi-settings.json
COPY lib/                                 /opt/kiwi/lib/

ENV PATH="/opt/kiwi/bin:$PATH"

ENTRYPOINT ["kiwi"]
