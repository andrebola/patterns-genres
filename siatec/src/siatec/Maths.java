package siatec;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.NavigableSet;
import java.util.TreeSet;

public class Maths {

	/**
	 * Computes the greatest common divisor of its two Integer arguments using
	 * Euclid's algorithm, as described in Cormen, Leiserson and Rivest (2000),
	 * p.810.
	 *
	 * @param a an Integer object
	 * @param b an Integer object
	 * @return the greatest common divisor of the two Integer arguments.
	 */
	public static Long gcd(Long a, Long b) {
		Long absA = Math.abs(a);
		Long absB = Math.abs(b);
		if (absB.equals(0l))
			return absA;
		else
			return gcd(absB, mod(absA, absB));
	}

	public static Integer gcd(Integer a, Integer b) {
		Integer absA = Math.abs(a);
		Integer absB = Math.abs(b);
		if (absB.equals(0))
			return absA;
		else
			return gcd(absB, mod(absA, absB));
	}

	/**
	 * Calculates the GCD of a set of integers, using Euclid's
	 * algorithm recursively:
	 * gcd(a,b,c,d) = gcd(a,gcd(b,c,d))
	 * 
	 * See Cormen, Leiserson, Rivest and Stein (2009, p.939).
	 * @param ints
	 * @return
	 */
	public static Integer gcd(Integer... ints) {
		if (ints == null || ints.length == 0) return null;
		if (ints.length == 1) return ints[0];
		if (ints.length == 2) return gcd(ints[0],ints[1]);
		Integer[] rest = Arrays.copyOfRange(ints, 1, ints.length);
		return gcd(ints[0],gcd(rest));
	}

	public static Long gcd(Long... longs) {
		if (longs == null || longs.length == 0) return null;
		if (longs.length == 1) return longs[0];
		if (longs.length == 2) return gcd(longs[0],longs[1]);
		Long[] rest = Arrays.copyOfRange(longs, 1, longs.length);
		return gcd(longs[0],gcd(rest));
	}

	
	/**
	 * Returns the largest integer less than or equal to a/b.
	 * @param a an Integer object
	 * @param b an Integer object
	 * @return an Integer object representing the largest integer less than or equal to a/b.
	 * @throws IllegalArgumentException if b is 0.
	 */
	public static Long floor(Long a, Long b) throws IllegalArgumentException {
		if (b.equals(0l))
			throw new IllegalArgumentException("Second argument to floor cannot be zero.");
		Long a2 = a, b2 = b;
		if (b < 0l) {
			b2 = -b;
			a2 = -a;
		}
		Long r = a2/b2; 
		/* This truncates if b2 does not divide a2, so if
		 * a2 is negative, r will be one greater than the
		 * desired result. Therefore...
		 */
		if (a%b != 0l && a2 < 0l) r--;
		return r;
	}

	public static Integer floor(Integer a, Integer b) throws IllegalArgumentException {
		if (b.equals(0))
			throw new IllegalArgumentException("Second argument to floor cannot be zero.");
		Integer a2 = a, b2 = b;
		if (b < 0) {
			b2 = -b;
			a2 = -a;
		}
		Integer r = a2/b2; 
		/* This truncates if b2 does not divide a2, so if
		 * a2 is negative, r will be one greater than the
		 * desired result. Therefore...
		 */
		if (a%b != 0 && a2 < 0) r--;
		return r;
	}

	/**
	 * Returns the least positive residue of a modulo b using formula 33.2 on
	 * p.803 of Cormen, Leiserson and Rivest (2000). Note that this method does
	 * NOT return the same value as the standard Java % operator!
	 *
	 * @param a
	 *            must be a Long
	 * @param b
	 *            must be a Long
	 * @return the least positive residue of a modulo b.
	 * @throws IllegalArgumentException
	 *             if b is zero.
	 */
	public static Long mod(Long a, Long b)
	throws IllegalArgumentException {
		if (b.equals(0l))
			throw new IllegalArgumentException(
					"Second argument to mod must not be zero.");
		return a - (b * floor(a,b));
	}

	public static Integer mod(Integer a, Integer b)
	throws IllegalArgumentException {
		if (b.equals(0))
			throw new IllegalArgumentException(
					"Second argument to mod must not be zero.");
		return a - (b * floor(a,b));
	}

	public static int getHighestPrimeFactor(int n) {
		int i;
		for (i = n; i > 0 && !(isPrime(i) && mod(n,i).equals(0)); i--);
		return i;
	}
	
	public static Boolean isPrime(int i) {
		if (i == 1) return true;
		for(int j = 2; j <= Math.sqrt(i); j++)
			if (mod(i,j) == 0) return false;
		return true;
	}

	public static ArrayList<Integer> factorize(Integer n) {
		int p;
		if (n.equals(0)) return null;
		ArrayList<Integer> output = new ArrayList<Integer>();
		do {
			p = Maths.getHighestPrimeFactor(n);
			output.add(p);
			n /= p;
		} while (p != 1);
		return output;
	}
	
	/**
	 * Returns least common multiple (lcm) of the integers a and b.
	 * 
	 * Uses formula lcm(a,b)=ab/gcd(a,b)
	 * 
	 * @param a, b are integers
	 * @return
	 */
	public static Integer lcm(Integer a, Integer b) {
		return (a*b)/gcd(a,b);
	}
	
	public static Long lcm(Long a, Long b) {
		return (a*b)/gcd(a,b);
	}
	
	/**
	 * Returns the least common multiple (lcm) of a sequence of ints.
	 * @param array of ints
	 */
	public static Integer lcm(Integer... nums) {
		if (nums.length == 1)
			return nums[0];
		if (nums.length == 2)
			return lcm(nums[0],nums[1]);
		Arrays.sort(nums);
		return lcm(nums[0],lcmAlreadySorted(Arrays.copyOfRange(nums, 1, nums.length)));
	}
	
	private static Integer lcmAlreadySorted(Integer... sortedNums) {
		if (sortedNums.length == 1)
			return sortedNums[0];
		if (sortedNums.length == 2)
			return lcm(sortedNums[0],sortedNums[1]);
		return lcm(sortedNums[0],lcmAlreadySorted(Arrays.copyOfRange(sortedNums, 1, sortedNums.length)));
	}
	
	public static Long lcm(Long... nums) {
		if (nums.length == 1)
			return nums[0];
		if (nums.length == 2)
			return lcm(nums[0],nums[1]);
		Arrays.sort(nums);
		return lcm(nums[0],lcmAlreadySorted(Arrays.copyOfRange(nums, 1, nums.length)));
	}
	
	private static Long lcmAlreadySorted(Long... sortedNums) {
		if (sortedNums.length == 1)
			return sortedNums[0];
		if (sortedNums.length == 2)
			return lcm(sortedNums[0],sortedNums[1]);
		return lcm(sortedNums[0],lcmAlreadySorted(Arrays.copyOfRange(sortedNums, 1, sortedNums.length)));
	}

	public static Long lcm(TreeSet<Long> sortedNums) {
		if (sortedNums.size() == 1)
			return sortedNums.first();
		if (sortedNums.size() == 2)
			return lcm(sortedNums.first(),sortedNums.tailSet(sortedNums.first(),false).first());
		return lcm(sortedNums.first(),lcm((TreeSet<Long>)(sortedNums.tailSet(sortedNums.first(),false))));
	}


	
	public static void main(String[] args) {
		System.out.println(lcm(3,4,8));
		
		
//		//Test mod
//		for (long a = -10; a < 10; a++)
//			for (long b = -10; b < 10; b++) {
//				try {
//					System.out.println(""+b+" mod "+a+" = "+mod(b,a));
//				} catch(IllegalArgumentException e) {
//					System.out.println(e.getMessage());
//				}
//			}
//
//		//Test floor
//		for (long a = -10; a < 10; a++)
//			for (long b = -10; b < 10; b++) {
//				try {
//					System.out.println("floor("+a+","+b+") = "+floor(a,b));
//				} catch(IllegalArgumentException e) {
//					System.out.println(e.getMessage());
//				}
//			}
//
//		//Test gcd
//		for (long a = -10; a < 10; a++)
//			for (long b = -10; b < 10; b++) {
//				try {
//					System.out.println("gcd("+a+","+b+") = "+gcd(a,b));
//				} catch(IllegalArgumentException e) {
//					System.out.println(e.getMessage());
//				}
//			}
//		
//		//Test isPrime
//		System.out.println("Primes less than 100");
//		for(int i = 0; i < 100; i++) 
//			if (isPrime(i))
//				System.out.println(i);
//		
//		//Test or highest prime factors
//		for(int i = 0; i < 100; i++)
//			System.out.println(""+i+" "+getHighestPrimeFactor(i));
//		
//		//Test factorize
//		for(int i = 0; i < 100; i++)
//			System.out.println(""+i+" "+factorize(i));
//		
//		//Test gcd with list of numbers
//		System.out.println(gcd(9,15,30));
	}

	public static double log2(long l) {
		return Math.log10(l)/Math.log10(2.0);
	}


}

