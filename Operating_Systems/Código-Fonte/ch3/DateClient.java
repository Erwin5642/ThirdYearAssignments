/**
 * Programa cliente solicitando a data atual ao servidor.
 *
 * Figure 3.22
 *
 * @author Silberschatz, Gagne and Galvin
 * Fundamentos de Sistemas Operacionais – Nona Edição
 */ 

import java.net.*;
import java.io.*;

public class DateClient
{
	public static void main(String[] args)  {
		try {
			// isso poderia ser alterado para um nome ou endereço IP diferente do pertencente ao hospedeiro local
			Socket sock = new Socket("127.0.0.1",6013);
			InputStream in = sock.getInputStream();
			BufferedReader bin = new BufferedReader(new InputStreamReader(in));

			String line;
			while( (line = bin.readLine()) != null)
				System.out.println(line);
				
			sock.close();
		}
		catch (IOException ioe) {
				System.err.println(ioe);
		}
	}
}
