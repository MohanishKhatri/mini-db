# Base image
FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y build-essential && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make build

# Final image
FROM ubuntu:24.04

WORKDIR /app

COPY --from=build /app/server ./server
RUN chmod +x ./server
CMD ["./server"]
