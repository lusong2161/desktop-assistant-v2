using Microsoft.VisualStudio.TestTools.UnitTesting;
using SmartAssistant.UI.ViewModels;
using SmartAssistant.UI.Services;

namespace SmartAssistant.UI.Tests
{
    [TestClass]
    public class UITests
    {
        [TestMethod]
        public void TestMainViewModelInitialization()
        {
            var viewModel = new MainViewModel();
            Assert.IsNotNull(viewModel);
        }

        [TestMethod]
        public void TestChatViewModelInitialization()
        {
            var viewModel = new ChatViewModel();
            Assert.IsNotNull(viewModel);
        }

        [TestMethod]
        public void TestDocumentViewModelInitialization()
        {
            var viewModel = new DocumentViewModel();
            Assert.IsNotNull(viewModel);
        }

        [TestMethod]
        public void TestFileTransferViewModelInitialization()
        {
            var viewModel = new FileTransferViewModel();
            Assert.IsNotNull(viewModel);
        }
    }
}
