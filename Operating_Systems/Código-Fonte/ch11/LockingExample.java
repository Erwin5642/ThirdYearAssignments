/**
  * Um programa simples que demonstra o bloqueio de arquivos.
  * Esse programa adquire um lock exclusivo na
 * primeira metade do arquivo e um lock compartilhado na segunda metade.
 *
  * Uso
 *	java LockingExample <input file>
 *
 * Figure 11.2
 *
 * @author Gagne, Galvin, Silberschatz
  * Fundamentos de Sistemas Operacionais – Nona Edição
 * Copyright John Wiley & Sons - 2013.
 */

import java.io.*;
import java.nio.channels.*;

public class LockingExample {
	public static final boolean EXCLUSIVE = false;
	public static final boolean SHARED = true;

   public static void main(String args[]) throws IOException {
	if (args.length != 1) {
		System.err.println("Usage: java LockingExample <input file>");
		System.exit(0);
	}

	FileLock sharedLock = null;
	FileLock exclusiveLock = null;

	try {
     		RandomAccessFile raf = new RandomAccessFile(args[0], "rw"); 

		// obtém o canal do arquivo
     		FileChannel channel = raf.getChannel();

		System.out.println("trying to acquire lock ...");
		// isso bloqueia a primeira metade do arquivo – exclusivo
		exclusiveLock = channel.lock(0, raf.length()/2, EXCLUSIVE);
		System.out.println("lock acquired ...");

		/**
		 * Agora modifica os dados. . .
		 */

		try {
			// entra em suspensão por 10 segundos
			Thread.sleep(10000);
		}
		catch (InterruptedException ie) { }

		// libera o lock
		exclusiveLock.release();
		System.out.println("lock released ...");

		// isso bloqueia a segunda metade do arquivo – compartilhado
		sharedLock = channel.lock(raf.length()/2 + 1, raf.length(), SHARED);
		
		/**
		  * Agora lê os dados. . .
		 */

		// libera o lock
		exclusiveLock.release();
	} catch (java.io.IOException ioe) {
		System.err.println(ioe);
	}
      	finally {
		if (exclusiveLock != null)
       			exclusiveLock.release();
		if (sharedLock != null)
       			sharedLock.release();
     	}
   }
}

