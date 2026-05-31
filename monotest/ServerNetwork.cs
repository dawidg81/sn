using System;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.IO.Compression;
using System.Text;

class Level
{
    public short SizeX { get; set; }
    public short SizeY { get; set; }
    public short SizeZ { get; set; }
    public byte[] Blocks { get; set; }
}

class ServerNetwork
{
    const int PORT = 25565;
    static Random random = new Random();

    static void SendLevel(NetworkStream stream, Level level)
    {
        int x = level.SizeX;
        int y = level.SizeY;
        int z = level.SizeZ;
        int totalBlocks = x * y * z;

        // Create level data with size prefix (big-endian 4-byte int)
        byte[] levelData = new byte[4 + totalBlocks];
        levelData[0] = (byte)((totalBlocks >> 24) & 0xFF);
        levelData[1] = (byte)((totalBlocks >> 16) & 0xFF);
        levelData[2] = (byte)((totalBlocks >> 8) & 0xFF);
        levelData[3] = (byte)(totalBlocks & 0xFF);

        Buffer.BlockCopy(level.Blocks, 0, levelData, 4, totalBlocks);

        // Compress using gzip
        byte[] compressed;
        using (MemoryStream ms = new MemoryStream())
        {
            using (GZipStream gs = new GZipStream(ms, CompressionMode.Compress, false))
            {
                gs.Write(levelData, 0, levelData.Length);
            }
            compressed = ms.ToArray();
        }

        Console.WriteLine("Compressed size: {0} bytes", compressed.Length);

        // Send init packet
        byte[] initPacket = new byte[] { 0x02 };
        stream.Write(initPacket, 0, 1);

        // Send level data in chunks
        int offset = 0;
        int totalSize = compressed.Length;

        while (offset < totalSize)
        {
            byte[] chunkPacket = new byte[1028];
            int chunkLen = Math.Min(1024, totalSize - offset);
            byte percent = (byte)(((offset + chunkLen) * 100) / totalSize);

            chunkPacket[0] = 0x03;
            chunkPacket[1] = (byte)((chunkLen >> 8) & 0xFF);
            chunkPacket[2] = (byte)(chunkLen & 0xFF);
            Buffer.BlockCopy(compressed, offset, chunkPacket, 3, chunkLen);
            // Bytes 3 + chunkLen to 1026 are already 0x00 (padding)
            chunkPacket[1027] = percent;

            stream.Write(chunkPacket, 0, 1028);
            offset += chunkLen;
        }

        // Send final packet with dimensions
        byte[] finalPacket = new byte[7];
        finalPacket[0] = 0x04;
        WriteU16BE(finalPacket, 1, (ushort)x);
        WriteU16BE(finalPacket, 3, (ushort)y);
        WriteU16BE(finalPacket, 5, (ushort)z);
        stream.Write(finalPacket, 0, 7);
    }

    static void WriteU16BE(byte[] buf, int offset, ushort value)
    {
        buf[offset] = (byte)((value >> 8) & 0xFF);
        buf[offset + 1] = (byte)(value & 0xFF);
    }

    static void Main()
    {
        TcpListener listener = new TcpListener(IPAddress.Any, PORT);
        listener.Start();

        Console.WriteLine("Server ready");

        while (true)
        {
            try
            {
                TcpClient client = listener.AcceptTcpClient();
                NetworkStream stream = client.GetStream();

                // Read client packet
                byte[] buffer = new byte[131];
                int bytesRead = stream.Read(buffer, 0, 131);

                if (bytesRead < 131)
                {
                    client.Close();
                    continue;
                }

                byte packetId = buffer[0];
                byte protocolVersion = buffer[1];
                string username = Encoding.ASCII.GetString(buffer, 2, 63).TrimEnd('\0');
                string verificationKey = Encoding.ASCII.GetString(buffer, 66, 63).TrimEnd('\0');
                byte unused = buffer[130];

                Console.WriteLine("New client connected");
                Console.WriteLine("Their username is {0}", username);
                Console.WriteLine("They are identifying with {0}", verificationKey);

                // Create and populate level
                Level level = new Level
                {
                    SizeX = 256,
                    SizeY = 64,
                    SizeZ = 256
                };

                int totalBlocks = level.SizeX * level.SizeY * level.SizeZ;
                level.Blocks = new byte[totalBlocks];

                for (int i = 0; i < totalBlocks; i++)
                {
                    level.Blocks[i] = (byte)(random.Next(2));
                }

                SendLevel(stream, level);

                client.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine("Error: {0}", e.Message);
            }
        }
    }
}
