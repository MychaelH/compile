FROM gcc:10.2
WORKDIR /app/
COPY main.cpp ./
RUN g++ main.cpp -o main
RUN chmod +x main