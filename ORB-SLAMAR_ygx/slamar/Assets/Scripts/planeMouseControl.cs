using UnityEngine;
using System.Collections;
using System.IO;

public class planeMouseControl : MonoBehaviour {

	float modelScaleX;
	float modelScaleZ;

	float rotateX;
	float rotateY;
	float rotateZ;

	// Use this for initialization
	void Start () {

		float[] trans = new float[9];
		StreamReader sr = new StreamReader ("./plane.txt");
		for (int i = 0; i < 9; i++)
		{
			string s = sr.ReadLine ();
			trans[i] = float.Parse(s);
		}
		sr.Close ();
		transform.position = new Vector3 (trans[0], trans[1], trans[2]);
		transform.rotation = Quaternion.Euler (trans[3], trans[4], trans[5]);
		transform.localScale = new Vector3 (trans[6], trans[7], trans[8]);
	
		rotateX = trans[3];
		rotateY = trans[4];
		rotateZ = trans[5];

		modelScaleX = trans[6];
		modelScaleZ = trans[8];

		StartCoroutine(OnMouseDown());

	}

	IEnumerator OnMouseDown()
	{
		Vector3 screenSpace = Camera.main.WorldToScreenPoint(transform.position);
		Vector3 offset = transform.position - Camera.main.ScreenToWorldPoint(new Vector3(Input.mousePosition.x, Input.mousePosition.y, screenSpace.z));
		while (Input.GetMouseButton(0))
		{
			Vector3 curScreenSpace = new Vector3(Input.mousePosition.x, Input.mousePosition.y, screenSpace.z);
			Vector3 curPosition = Camera.main.ScreenToWorldPoint(curScreenSpace) + offset;
			transform.position = curPosition;

			if (Input.GetAxis("Mouse ScrollWheel") < 0)
			{
				modelScaleX -= 0.0128f;
				modelScaleZ -= 0.008f;

				transform.localScale = new Vector3(modelScaleX, 1.0f, modelScaleZ);
			}
			if (Input.GetAxis("Mouse ScrollWheel") > 0)
			{
				modelScaleX += 0.0128f;
				modelScaleZ += 0.008f;

				transform.localScale = new Vector3(modelScaleX, 1.0f, modelScaleZ);
			}

			yield return new WaitForFixedUpdate();
		}
	}
	
	// Update is called once per frame
	void Update () {
		
		if (Input.GetMouseButton(1))
		{
			Ray ray = Camera.main.ScreenPointToRay (Input.mousePosition);
			RaycastHit hitInfo;
			if (Physics.Raycast (ray, out hitInfo))
			{
				GameObject gameObj = hitInfo.transform.gameObject;

				rotateX += Input.GetAxis ("Mouse X") * 5.0f;
				rotateY += Input.GetAxis ("Mouse Y") * 5.0f;

				if (Input.GetAxis ("Mouse ScrollWheel") < 0) {
					rotateZ += 2.0f;
				}
				if (Input.GetAxis ("Mouse ScrollWheel") > 0) {
					rotateZ -= 2.0f;
				}

				var rotation = Quaternion.Euler (-rotateY, -rotateX, rotateZ);
				gameObj.transform.rotation = rotation;
			}
		}

	}
}
