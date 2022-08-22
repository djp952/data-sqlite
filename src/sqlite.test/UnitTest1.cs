using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using zuki.data.sqlite;

namespace sqlite.test
{
	[TestClass]
	public class UnitTest1
	{
		[TestMethod]
		public void TestMethod1()
		{
			SqliteConnection conn = new SqliteConnection();
			conn.Dispose();
		}
	}
}
