using UnityEngine;
using System.Collections;

public class planeRender : MonoBehaviour {

	public MovieTexture MovTexture;
	public float MovieSize = 12;

	// Use this for initialization
	void Start () {
		GetComponent<Renderer>().material.mainTexture = MovTexture;
		MovTexture.loop = true;

		PlayMovie ();

	}
	
	// Update is called once per frame
	void Update () {
		//transform.localScale = new Vector3(3.5f,1,1.5f);
	}

	public void PlayMovie()
	{
		if (!MovTexture.isPlaying) {
			MovTexture.Play ();
		}
	}


	public void PauseMovie()
	{
		MovTexture.Pause();
	}

	public void StopMovie()
	{
		MovTexture.Stop();
	}
}
