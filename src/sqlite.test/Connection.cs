using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using zuki.data.sqlite;

namespace sqlite.test
{
	[TestClass]
	public class Connection
	{
		[TestMethod]
		public void Construct()
		{
			using(SqliteConnection conn = new SqliteConnection())
			{
				conn.ConnectionString = "Data Source=:memory:";
				conn.Open();
			}
		}
	}
}
