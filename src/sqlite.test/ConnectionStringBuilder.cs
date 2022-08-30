using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using zuki.data.sqlite;

namespace sqlite.test
{
	[TestClass]
	public class ConnectionStringBuilder
	{
		[TestMethod]
		public void Construct()
		{
			SqliteConnectionStringBuilder csb = new SqliteConnectionStringBuilder
			{
				AllowExtensions = true
			};
		}
	}
}
