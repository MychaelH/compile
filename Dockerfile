FROM gcc:10.2
WORKDIR /app/
COPY main.cpp ./
COPY getword.h ./
COPY symbol_table.h ./
RUN g++ main.cpp -o main
RUN chmod +x main