FROM gcc:latest
WORKDIR .
COPY ./server server
COPY ../common common
COPY ../build.sh build.sh
RUN chmod +x build.sh
RUN ./build.sh server
CMD ["./run_server"]