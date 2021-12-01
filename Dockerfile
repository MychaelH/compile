FROM gcc:10.2
WORKDIR /app/
COPY main.cpp ./
COPY getword.h ./
COPY symbol_table.h ./
COPY Output_region.h ./
RUN g++ main.cpp -o main
RUN chmod +x main