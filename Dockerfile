FROM debian:bullseye

RUN apt-get update && apt-get install -y build-essential liballegro4-dev && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY Mandalay.c settings.txt Makefile .

RUN make

ENTRYPOINT ["./Mandalay"]
