FROM frolvlad/alpine-gxx
WORKDIR /app/
COPY main.cpp ./
RUN g++ main.cpp -o main
RUN chmod +x main