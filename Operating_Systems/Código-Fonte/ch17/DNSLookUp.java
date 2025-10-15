/**
  * Um programa simples que demonstra a pesquisa de DNS
 *
 * Uso:
 *	java DNSLookUp <IP Name>
 *
 * Figure 16.5

 * @author Gagne, Galvin, Silberschatz
  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013.
 *
 */

import java.io.*;
import java.net.*;

public class DNSLookUp
{
	public static void main(String[] args) {
		if (args.length != 1) {
			System.err.println("Usage: java DNSLookUp <IP name>");
			System.exit(0);
		}

		InetAddress hostAddress;

		try {
			hostAddress = InetAddress.getByName(args[0]);
			System.out.println(hostAddress.getHostAddress());
		}
		catch (UnknownHostException uhe) {
			System.err.println("Unknown host: " + args[0]);
		}
	}
}
