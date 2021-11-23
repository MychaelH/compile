FROM gcc:10.2
WORKDIR /myapp/src/dst
COPY main.cpp ./
RUN g++ main.cpp -o main
RUN chmod +x main