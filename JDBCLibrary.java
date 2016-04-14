package jdbcLibrary;

import java.sql.*;
import java.util.Scanner;
import java.util.Vector;

public class JDBCLibrary {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		try{
			//establish connection to server
			
			Connection myConn = DriverManager.getConnection("jdbc:mysql://127.0.0.1:3306/library", "root", "Notgoaway00");
			//get variables I'll need to run menu
			
			String input;
			Scanner sc = new Scanner(System.in);
			
			//menu loop
			
			while(true){
				//display menu
				
				System.out.println("Please choose an option from the menu:");
				System.out.println("A. Create a table");
				System.out.println("B. Add an entry/entries");
				System.out.println("C. Delete an entry/entries");
				System.out.println("D. List a table");
				System.out.println("1. Add a new book to the library");
				System.out.println("2. Add a new user");
				System.out.println("3. List library users with books they checked out");
				System.out.println("4. Check out a book");
				System.out.println("5. Return a book");
				System.out.println("6. Exit");
				
				//get and process choice
				
				input = sc.nextLine();
				
				switch(input){
				case "A": createTable(myConn); break;
				case "B": insert(myConn); break;  
				case "C": delete(myConn); break;
				case "D": listTable(myConn); break;
				case "1": addBook(myConn); break;
				case "2": addUser(myConn); break;
				case "3": listCheckedOut(myConn); break;
				case "4": checkOut(myConn); break;
				case "5": returnBook(myConn); break;
				case "6": myConn.close(); sc.close(); exit(); break;
				}
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
	public static void createTable(Connection c){
		try{
			//declare required variables
			
			Scanner sc = new Scanner(System.in);
			String command = "CREATE TABLE "; //sql command to run
			String input;
			int numColumns; //number of columns is user defined
			
			//get table name
			
			System.out.println("Enter the name of the table:");
			command = command + sc.nextLine() + "( ";
			
			//get columns
			
			System.out.println("Enter number of columns in this table:");
			numColumns = sc.nextInt();
			sc.nextLine();
			
			for(int i = 0; i <= numColumns - 1; i++){
				System.out.println("Enter the name of a column:");
				input = sc.nextLine();
				if(i == numColumns - 1)
					command = command + input + " TEXT NOT NULL)";
				else
					command = command + input + " TEXT NOT NULL, ";
			}
			
			//sc.close();
			
			//run command using the full string
			
			Statement stmt = c.createStatement();
			stmt.executeUpdate(command);
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	public static void insert(Connection c){
		try{
			
			//get name of table to insert to
			
			Scanner sc = new Scanner(System.in);
			DatabaseMetaData metadata = c.getMetaData();
			ResultSet rs;
			String command = "insert into ";
			String tableName = ""; //need to check if table name entered exists
			Vector<String> values = new Vector<String>();
			boolean validName = false;
			
			
			while(!validName){
				System.out.println("Enter the name of the table you wish to modify: ");
				tableName = sc.nextLine();
				
				rs = metadata.getTables(null, null, tableName, null);
				if(rs.next())
					validName = true;
				else
					System.out.println("That table does not exist. Please try again.");
			}
			
			command = command + tableName + " (";
			
			//get values to insert and build statement
			
			Statement stmt = c.createStatement();
			rs = stmt.executeQuery("SELECT * FROM " + tableName);
			ResultSetMetaData rsmd = rs.getMetaData();
			int columnCount = rsmd.getColumnCount();
			
			for(int i = 1; i <= columnCount; i++){
				String columnName = rsmd.getColumnName(i);
				if(i == columnCount)
					command = command + columnName + ") VALUES(";
				else
					command = command + columnName + ", ";
				System.out.println("Enter " + columnName + ": ");
				values.add(sc.nextLine());
			}
			
			for(String value: values){
				if(value == values.lastElement()) //possible source of bug in future
					command = command + "'" + value + "'" + ")"; 
				else
					command = command + "'" + value + "'" + ", ";
			}
			
			//insert new values into database
			
			stmt.executeUpdate(command);
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	public static void delete(Connection c){ //use condition to specify if or not multiple values are to be deleted
		try{
			
			//get table name and see if it exists
			
			Scanner sc = new Scanner(System.in);
			String input = "";
			String command = "DELETE FROM ";
			boolean validName = false;
			DatabaseMetaData metadata = c.getMetaData();
			ResultSet rs = null;
			
			while(!validName){
				System.out.println("Enter the name of the table you wish to modify: ");
				input = sc.nextLine(); //user can enter more than one table
				
				rs = metadata.getTables(null, null, input, null);
				if(rs.next())
					validName = true;
				else
					System.out.println("That table does not exist. Please try again.");
			}
			
			command += input;
			
			//get condition for deletion, if any
			/*could have done a loop to delete one at a time
			 * but I felt that generic commands are back end so we can practically
			 * rely on some computer proficiency and can eschew niceties
			 */
			
			System.out.println("Condition for deletion? Y/N");
			input = sc.nextLine();
			if(input.charAt(0) == 'Y' || input.charAt(0) == 'y'){
				System.out.println("Enter condition:");
				input = sc.nextLine();
				command = command + " WHERE " + input;
			}
			else
				System.out.println("Deleting all records.");
				
			//execute deletion
			
			Statement stmt = c.createStatement();
			stmt.executeUpdate(command);
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	public static void listTable(Connection c){
		try{
			
			//get table name and see if it exists
			
			Scanner sc = new Scanner(System.in);
			String input = "";
			String command = "SELECT * FROM ";
			boolean validName = false;
			DatabaseMetaData metadata = c.getMetaData();
			ResultSet rs = null;
			
			while(!validName){
				System.out.println("Enter the name of the table you wish to print: ");
				input = sc.nextLine();
				
				rs = metadata.getTables(null, null, input, null);
				if(rs.next())
					validName = true;
				else
					System.out.println("That table does not exist. Please try again.");
			}
			
			command += input;
			
			//executeQuery on that table
			
			Statement stmt = c.createStatement();
			rs = stmt.executeQuery(command);
			ResultSetMetaData rsmd = rs.getMetaData();
			int columnCount = rsmd.getColumnCount();
			
			//print all column names
			for (int i = 1; i <= columnCount; i++ ) 
				  System.out.print(rsmd.getColumnName(i) + "\t\t");
			System.out.println("");
			
			//print rest of table
			
			while(rs.next()){
				for(int i = 1; i <= columnCount; i++){
					System.out.print(rs.getString(i) + "\t");
				}
				System.out.println("");
			}
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	public static void addBook(Connection c){
		try{
			
			//prepare statement
			
			Statement stmt = c.createStatement();
			String command = "";
			boolean wantsToAdd = true;
			Scanner sc = new Scanner(System.in);
			String input = "";
			
			//main loop
			
			while(wantsToAdd){
				
				command = "INSERT INTO books (author,title,callNo) VALUES (";
				//get values to input
				
				System.out.println("Enter the book's author:");
				input = sc.nextLine();
				command = command + "'" + input + "', "; 
				
				System.out.println("Enter the title: ");
				input = sc.nextLine();
				command = command + "'" + input + "', ";
				
				System.out.println("Enter the call number:");
				input = sc.nextLine();
				command = command + "'" + input + "')";
				
				//execute command
				stmt.executeUpdate(command);
				
				//query for another input
				System.out.println("Enter another book? Y/N");
				input = sc.nextLine();
				if(input.charAt(0) == 'N' || input.charAt(0) == 'n')
					wantsToAdd = false;
			}
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	public static void addUser(Connection c){
		try{
			//prepare statement
			
			Statement stmt = c.createStatement();
			String command = "";
			boolean wantsToAdd = true;
			Scanner sc = new Scanner(System.in);
			String input = "";
			
			//main loop
			while(wantsToAdd){
				
				command = "INSERT INTO people (SSN, name) VALUES (";
				
				//get values
				
				System.out.println("Enter the person's SSN:");
				input = sc.nextLine();
				command = command + "'" + input + "',";
				
				System.out.println("Enter the name of the person:");
				input = sc.nextLine();
				command = command + "'" + input + "')";
				
				//execute command
				stmt.executeUpdate(command);
				
				//query for another input
				System.out.println("Enter another person? Y/N");
				input = sc.nextLine();
				if(input.charAt(0) == 'N' || input.charAt(0) == 'n')
					wantsToAdd = false;
			}
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	public static void listCheckedOut(Connection c){
		try{
			Statement stmt = c.createStatement();
			ResultSet rs;
			ResultSetMetaData rsmd;
			int columnCount;
			
			//query for users with checked out books
			
			rs = stmt.executeQuery("SELECT name, author, title " +
									" FROM checkedOutBooks JOIN people ON checkedOutBooks.SSN = people.SSN " +
									" JOIN books ON checkedOutBooks.callNo = books.callNo");
			rsmd = rs.getMetaData();
			columnCount = rsmd.getColumnCount();
			
			//print all column names
			
			for (int i = 1; i <= columnCount; i++ ) 
				  System.out.print(rsmd.getColumnName(i) + "\t");
			System.out.println("");
			
			//print rest of table
			
			while(rs.next()){
				for(int i = 1; i <= columnCount; i++){
					System.out.print(rs.getString(i) + "\t");
				}
				System.out.println("");
			}
			
			System.out.println(""); //empty line for formatting
			
			//query for users with no checked out books
			
			rs = stmt.executeQuery("SELECT name FROM people WHERE SSN NOT IN (SELECT SSN FROM checkedOutBooks)");
			
			//display users
			
			System.out.println("People with no checked out books:");
			while(rs.next()){
				System.out.println(rs.getString("name"));
			}
			System.out.println(""); //for formatting
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	public static void checkOut(Connection c){ 
		try{
			Statement stmt = c.createStatement();
			String command = "";
			String input = "";
			ResultSet rs = null;
			Scanner sc = new Scanner(System.in);
			String ssn = "";
			String callNo = "";
			boolean validSSN = false;
			boolean wantsCheckout = true;
			
			while(!validSSN){
				
				//get SSN from user
				
				System.out.println("Enter person's SSN:");
				ssn = sc.nextLine();
				
				//verify SSN exists
				rs = stmt.executeQuery("SELECT SSN FROM people WHERE SSN = " + "'" + ssn + "'");
				if(rs.next())
					validSSN = true;
				else
					System.out.println("This SSN does not exist. Please try again.");
			}
			
			while(wantsCheckout){
				
				command = "INSERT INTO checkedOutBooks (SSN,callNo) VALUES (" + "'" + ssn + "', ";
				
				//get callNo
				
				System.out.println("Enter book's call number:");
				callNo = sc.nextLine();
				
				//verify callNo exists 
				
				rs = stmt.executeQuery("SELECT callNo FROM books WHERE callNo = " + "'" + callNo + "'");
				if(!rs.next()){
					System.out.println("This callNo does not exist. Please try again.");
					continue;
				}
				
				//verify callNo not already checked out
				
				rs = stmt.executeQuery("SELECT callNo FROM checkedOutBooks WHERE callNo = " + "'" + callNo + "'");
				if(rs.next()){
					System.out.println("This book has already been checked out. Check out different book? Y/N");
					String temp = sc.nextLine();
					if(temp.charAt(0) == 'N' || temp.charAt(0) == 'n')
						break;
					else continue;
				}
			
				//update list of checked out books
				
				command = command + "'" + callNo + "')";
				stmt.executeUpdate(command);
				
				//query for another check out
				System.out.println("Check out another book? Y/N");
				input = sc.nextLine();
				if(input.charAt(0) == 'N' || input.charAt(0) == 'n')
					wantsCheckout = false;
			}
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	public static void returnBook(Connection c){
		try{
			Statement stmt = c.createStatement();
			ResultSet rs = null;
			Scanner sc = new Scanner(System.in);
			String command = "";
			String ssn = "";
			String callNo = "";
			String input = "";
			boolean validSSN = false;
			boolean wantsReturn = true;
			
			while(!validSSN){
				
				//get SSN from user
				
				System.out.println("Enter person's SSN:");
				ssn = sc.nextLine();
				
				//verify SSN exists
				
				rs = stmt.executeQuery("SELECT SSN FROM people "
										+ "WHERE SSN = " + "'" + ssn + "'");
				
				if(rs.next())
					validSSN = true;
				else
					System.out.println("We cannot find record of this SSN. Please try again.");
				
				//verify SSN has checked out a book
				rs = stmt.executeQuery("SELECT SSN FROM checkedoutbooks "
						+ "WHERE SSN = " + "'" + ssn + "'");

				if(rs.next())
					validSSN = true;
				else
					System.out.println("We cannot find record of this SSN. Please try again.");
			}
			
			while(wantsReturn){
				command = "DELETE FROM checkedOutBooks WHERE SSN = '" + ssn + "' AND callNo = ";
				//get callNo
				
				System.out.println("Enter call number of book you wish to return:");
				callNo = sc.nextLine();
				
				//verify callNo exists
				
				rs = stmt.executeQuery("SELECT callNo FROM books WHERE callNo = " + "'" + callNo + "'");
				
				if(!rs.next())
					System.out.println("This call number does not exist. Please try again.");
				
				//verify callNo checked out by SSN entered
				
				rs = stmt.executeQuery("SELECT callNo FROM checkedOutBooks WHERE SSN = " + "'" + ssn + "'" + " AND callNo = " + "'" + callNo + "'");
				if(!rs.next()){
					System.out.println("We have no record of this user checking out this book. Return another book? Y/N");
					input = sc.nextLine();
					if(input.charAt(0) == 'N' || input.charAt(0) == 'n')
						break;
					else continue;
				}
				
				command = command + "'" + callNo + "'";
				
				//remove entry from list of checked out books
				
				stmt.executeUpdate(command);
				
				//query for another return
				
				System.out.println("Return another book? Y/N");
				input = sc.nextLine();
				if(input.charAt(0) == 'N' || input.charAt(0) == 'n')
					wantsReturn = false;
			}
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	public static void exit(){
		System.out.println("Goodbye");
		System.exit(0);
	}

}
