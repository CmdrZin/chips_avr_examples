using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MsgQueue : MonoBehaviour {
	public Queue<string> comMsgQueue = new Queue<string>();
	public string qTop;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		// Monitor the message queue.
		if (!comMsgQueue.IsEmpty ()) {
			qTop = comMsgQueue.Dequeue ();
		}
	}
}

public class Queue<T>
{
	private const int DEFAULTQUEUESIZE = 100;
	private T[] data;
	private int head = 0, tail = 0;
	private int numElements = 0;

	public Queue()
	{
		this.data = new T[DEFAULTQUEUESIZE];
	}

	public Queue(int size)
	{
		if (size > 0) {
			this.data = new T[size];
		} else {
			throw new UnityException ("Size must be greater than zero");
		}
	}

	public void Enqueue(T item)
	{
		if (this.numElements == this.data.Length) {
			throw new UnityException ("Queue full");
		}

		this.data [this.head] = item;
		this.head++;
		this.head %= this.data.Length;
		this.numElements++;
	}

	public T Dequeue()
	{
		if (this.numElements == 0) {
			throw new UnityException ("Queue empty");
		}

		T queueItem = this.data [this.tail];
		this.tail++;
		this.tail %= this.data.Length;			// this causes tail to rap around to top.
		this.numElements--;
		return queueItem;
	}

	public bool IsEmpty()
	{
		if (this.numElements == 0) {
			return true;
		} else {
			return false;
		}
	}
}
