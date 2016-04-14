package puzzleSearch;
//Thomas Varney
//Primary code for A* Search
//This program attempts to solve the 19-puzzle using the A* Search algorithm
import java.util.Scanner;
import java.util.HashMap;
import java.util.Stack;

public class starSearch { 
	static Point[] coordinates = new Point[20]; //corresponding coordinates for each array index
	public static Node search(int[] startBoard) {
		//this method is the search algorithm itself
		MinPQ frontier = new MinPQ();
		HashMap<State, Node> createdNodes = new HashMap<State, Node>();
		Node currentNode = new Node(startBoard);
		Node[] children;
		Node storedNode; //node stored in hash map already
		//My explored set is not an explicit data structure because we keep track of created nodes anyway
		//Those created nodes have a boolean explored field which can be checked against
		
		currentNode.frontier(); //set frontier to true
		frontier.add(currentNode); //add start node to frontier
		createdNodes.put(currentNode.getBoardState(), currentNode); //add start node to created
		
		while(!frontier.isEmpty()) {
			currentNode = frontier.remove();
			children = currentNode.generateMoves();
			
			for(int i = 0; i < children.length; i++) {
				if(children[i].goalTest()) {
					return children[i]; //return solution
				}
				storedNode = createdNodes.get(children[i].getBoardState());
				if(storedNode != null) {
					if(storedNode.inFrontier() && storedNode.compareTo(children[i]) > 0) {
						storedNode.update(children[i]);
						frontier.update(storedNode);
					}
					continue;
				}
				else {
					children[i].frontier();
					frontier.add(children[i]);
					createdNodes.put(children[i].getBoardState(), children[i]);
				}
			}
			currentNode.explored();
		}
		return null; //fail state
	}
	public static void print(Node solution) {
		//if solution is null print failure message
		//else follow the chain up to the start node
		Stack<Node> printStack = new Stack<Node>();
		if(solution == null) {
			System.out.println("The program has failed to find a solution.");
		}
		else {
			while(solution != null) {
				printStack.push(solution);
				solution = solution.getParent();
			}
			System.out.println("Start State: ");
			while(!printStack.empty()) { printStack.pop().print(); }
		}
	}
	public static int distance(int[] board) {
		/* With the board state a 1-d array, the values 0-19 also define their proper position in the array.
		 * Each of these has a corresponding coordinate on the actual 2-d coordinate plane, given by the
		 * coordinates array, which is what is used for this Manhattan distance. Thus, the current index of the 1-d array
		 * defines the current coordinate of the value, while the value contained therein gives the corresponding
		 * goal coordinate. */
		int sum = 0;
		for(int i = 0; i < board.length; i++) {
			if(board[i] != 0) {
				sum += Math.abs(coordinates[i].getX() - coordinates[board[i]].getX()) 
						+ Math.abs(coordinates[i].getY() - coordinates[board[i]].getY());
			}
		}
		return sum;
	}
	private static void setCoordinates() {
		coordinates[0] = new Point(2,0);
		coordinates[1] = new Point(3,0);
		coordinates[2] = new Point(2,1);
		coordinates[3] = new Point(3,1);
		coordinates[4] = new Point(0,2);
		coordinates[5] = new Point(1,2);
		coordinates[6] = new Point(2,2);
		coordinates[7] = new Point(3,2);
		coordinates[8] = new Point(4,2);
		coordinates[9] = new Point(5,2);
		coordinates[10] = new Point(0,3);
		coordinates[11] = new Point(1,3);
		coordinates[12] = new Point(2,3);
		coordinates[13] = new Point(3,3);
		coordinates[14] = new Point(4,3);
		coordinates[15] = new Point(5,3);
		coordinates[16] = new Point(2,4);
		coordinates[17] = new Point(3,4);
		coordinates[18] = new Point(2,5);
		coordinates[19] = new Point(3,5);
	}
	public static void main(String[] args) {
		Scanner sc = new Scanner(System.in);
		Node solution;
		int[] startBoard = new int[20];
		
		setCoordinates(); //populate coordinates array;
						  //coordinates is primarily used for Manhattan Distance;
						  //is static because it would be more time and memory
						  //consuming to attach a point array to each state;
		
		//getting the array from standard input was attributed to a stack overflow page
		//I didn't need help with reading in stuff but rather with doing it all in one line expediently
		System.out.println("Enter the 20 numbers that form the initial board position.");
		Scanner init = new Scanner(sc.nextLine());
		
		for(int i = 0; i < 20; i++) {
			if(init.hasNextInt()) {
				startBoard[i] = init.nextInt();
			}
		}
		
		init.close();
		sc.close();
		
		solution = search(startBoard); //search returns goal state node
		print(solution); //print follows the chain up to the start node
	}
}
