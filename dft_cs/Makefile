TARGET = dft
SOURCE = DFT.cs WaveReader.cs
CSFLAGS = -out:$(TARGET)

all: $(TARGET)

$(TARGET): $(SOURCE)
	mcs $(CSFLAGS) $^

.PHONY: clean
clean:
	rm $(TARGET)
